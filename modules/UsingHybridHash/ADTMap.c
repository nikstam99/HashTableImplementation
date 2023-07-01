/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Hash Table με open addressing (linear probing)
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "ADTVector.h"
#include "ADTMap.h"
#include "stdio.h"


// Οι κόμβοι του map στην υλοποίηση με hash table, μπορούν να είναι σε 3 διαφορετικές καταστάσεις,
// ώστε αν διαγράψουμε κάποιον κόμβο, αυτός να μην είναι empty, ώστε να μην επηρεάζεται η αναζήτηση
// αλλά ούτε occupied, ώστε η εισαγωγή να μπορεί να το κάνει overwrite.
typedef enum {
	EMPTY, OCCUPIED
} State;

// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Χρησιμοποιούμε open addressing, οπότε σύμφωνα με την θεωρία, πρέπει πάντα να διατηρούμε
// τον load factor του  hash table μικρότερο ή ίσο του 0.5, για να έχουμε αποδoτικές πράξεις
#define MAX_LOAD_FACTOR 0.5

// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;
	Pointer value;
	State state;		// Μεταβλητή για να μαρκάρουμε την κατάσταση των κόμβων 
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	MapNode array;				// Ο πίνακας που θα χρησιμοποιήσουμε για το map (remember, φτιάχνουμε ένα hash table)
	Vector *chains;
	int capacity;				// Πόσο χώρο έχουμε δεσμεύσει.
	int size;					// Πόσα στοιχεία έχουμε προσθέσει
	CompareFunc compare;		// Συνάρτηση για σύγκριση δεικτών, που πρέπει να δίνεται απο τον χρήστη
	HashFunc hash_function;		// Συνάρτηση για να παίρνουμε το hash code του κάθε αντικειμένου.
	DestroyFunc destroy_key;	// Συναρτήσεις που καλούνται όταν διαγράφουμε έναν κόμβο απο το map.
	DestroyFunc destroy_value;
};

Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->array = malloc(map->capacity * sizeof(struct map_node));
	map->chains = malloc(map->capacity * sizeof(Vector));
	// Αρχικοποιούμε τους κόμβους που έχουμε σαν διαθέσιμους.
	for (int i = 0; i < map->capacity; i++) {
		map->array[i].state = EMPTY;
		map->chains[i] = NULL;
	}
	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map) {
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	MapNode old_array = map->array;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο. 
	int prime_no = sizeof(prime_sizes) / sizeof(int);	// το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++) {					// LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity)					// LCOV_EXCL_LINE
		map->capacity *= 2;								// LCOV_EXCL_LINE
		
		
	Vector *map_chains;
	map_chains = map->chains;
	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->array = malloc(map->capacity * sizeof(struct map_node));
	map->chains = malloc(map->capacity * sizeof(Vector));
	for (int i = 0; i < map->capacity; i++) {
		map->array[i].state = EMPTY;
		map->chains[i] = NULL;
	}
	// Τοποθετούμε τα entries που όντως περιέχουν ένα στοιχείο
	map->size = 0;
	for (int i = 0; i < old_capacity; i++) {
		if (old_array[i].state == OCCUPIED) {
			map_insert(map, old_array[i].key, old_array[i].value);
		}
		if (map_chains[i] != NULL) {
			for (VectorNode node = vector_first(map_chains[i]);
			node != VECTOR_EOF;
			node = vector_next(map_chains[i], node)) {
				MapNode N = vector_node_value(map_chains[i], node);
				map_insert(map, N->key, N->value);
			}
		}
	}
	//Αποδεσμεύουμε τον παλιό πίνακα ώστε να μήν έχουμε leaks
	free(old_array);
	free(map_chains);
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.
bool map_insert_in_hash(Map map, Pointer key, Pointer value) {
	// Σκανάρουμε το Hash Table μέχρι να βρούμε διαθέσιμη θέση για να τοποθετήσουμε το ζευγάρι,
	// ή μέχρι να βρούμε το κλειδί ώστε να το αντικαταστήσουμε.
	bool already_in_map = false;
	MapNode node = NULL;
	uint pos;
	uint start = map->hash_function(key) % map->capacity;
	uint stop = (start + 3) % map->capacity;
	for (pos = start;							// ξεκινώντας από τη θέση που κάνει hash το key
		pos != stop;	
		pos = (pos + 1) % map->capacity) {		// linear probing, γυρνώντας στην αρχή όταν φτάσουμε στη τέλος του πίνακα

		if (map->array[pos].state == EMPTY) {
			node = &map->array[pos];
		}
		else if (map->compare(map->array[pos].key, key) == 0) {
			already_in_map = true;
			node = &map->array[pos];						// βρήκαμε το key, το ζευγάρι θα μπει αναγκαστικά εδώ 
			break;	
		}
	}
		if (node == NULL) return false;
	
		if (!already_in_map) map->size++;
		
		else {
			// Αν αντικαθιστούμε παλιά key/value, τa κάνουμε destroy
			if (node->key != key && map->destroy_key != NULL)
				map->destroy_key(node->key);

			if (node->value != value && map->destroy_value != NULL)
				map->destroy_value(node->value);
		}
		node->key = key;
		node->state = OCCUPIED;
		node->value = value;

		return true;
}

void map_insert_in_vector(Map map, Pointer key, Pointer value){
	bool already_in_map = false;
	uint pos = map->hash_function(key) % map->capacity;
	MapNode node = &map->array[pos];
	if (map->chains[pos] != NULL) {
		for (int i = 0; i < vector_size(map->chains[pos]); i++) {
			MapNode N = vector_get_at(map->chains[pos], i);
			if (map->compare(N->key, key) == 0) {
				node = N;
				already_in_map = true;
			}
		}
		if (!already_in_map) {
			node->key = key;
			node->value = value;
			node->state = OCCUPIED;
			vector_insert_last(map->chains[pos], node);
		}
	}
	else {
		map->chains[pos] = vector_create(0, free);
		node->key = key;
		node->value = value;
		node->state = OCCUPIED;
		vector_insert_last(map->chains[pos], node);
	}
		if (!already_in_map) map->size++;
		
		else {
			// Αν αντικαθιστούμε παλιά key/value, τa κάνουμε destroy
			if (node->key != key && map->destroy_key != NULL)
				map->destroy_key(node->key);

			if (node->value != value && map->destroy_value != NULL)
				map->destroy_value(node->value);
		}
		node->key = key;
		node->state = OCCUPIED;
		node->value = value;
}


void map_insert(Map map, Pointer key, Pointer value) {
	// Σκανάρουμε το Hash Table μέχρι να βρούμε διαθέσιμη θέση για να τοποθετήσουμε το ζευγάρι,
	// ή μέχρι να βρούμε το κλειδί ώστε να το αντικαταστήσουμε.

	bool insert = map_insert_in_hash(map, key, value);
	if (insert);
	if (!insert) map_insert_in_vector(map, key, value);
	//if (insert) printf("1");
	//else printf("0");
	// Αν με την νέα εισαγωγή ξεπερνάμε το μέγιστο load factor, πρέπει να κάνουμε rehash.
	float load_factor = (float)(map->size) / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);
}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node == MAP_EOF)
		return false;

	// destroy
	if (map->destroy_key != NULL)
		map->destroy_key(node->key);
	if (map->destroy_value != NULL)
		map->destroy_value(node->value);

	map->size--;
	node->state = EMPTY;
	node->key = NULL;
	node->value = NULL;
	return true;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node != MAP_EOF)
		return node->value;
	else
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		if (map->array[i].state == OCCUPIED) {
			if (map->destroy_key != NULL)
				map->destroy_key(map->array[i].key);
			if (map->destroy_value != NULL)
				map->destroy_value(map->array[i].value);
		}
		if (map->chains[i] != NULL) {
			vector_destroy(map->chains[i]);
		}
	}

	free(map->array);
	free(map->chains);
	free(map);
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
	//Ξεκινάμε την επανάληψή μας απο το 1ο στοιχείο, μέχρι να βρούμε κάτι όντως τοποθετημένο
	for (int i = 0; i < map->capacity; i++)
		if (map->array[i].state == OCCUPIED)
			return &map->array[i];

	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	// Το node είναι pointer στο i-οστό στοιχείο του array, οπότε node - array == i  (pointer arithmetic!)
	if (node - map->array < map->capacity && node - map->array >= 0) {
		for (int i = node - map->array + 1; i < map->capacity; i++)
			if (map->array[i].state == OCCUPIED)
				return &map->array[i];
	}
	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	// Διασχίζουμε τον πίνακα, ξεκινώντας από τη θέση που κάνει hash το key, και για όσο δε βρίσκουμε EMPTY
	uint start = map->hash_function(key) % map->capacity;
	uint pos;
	for (pos = start;										// ξεκινώντας από τη θέση που κάνει hash το key
		pos != (start + 3) % map->capacity;							// αν φτάσουμε σε EMPTY σταματάμε
		pos = (pos + 1) % map->capacity) {						// linear probing, γυρνώντας στην αρχή όταν φτάσουμε στη τέλος του πίνακα

		// Μόνο σε OCCUPIED θέσεις, ελέγχουμε αν το key είναι εδώ
		if (map->array[pos].state == OCCUPIED && map->compare(map->array[pos].key, key) == 0)
			return &map->array[pos];
	}
	if (map->chains[start] != NULL) {
		for (VectorNode node = vector_first(map->chains[start]); 
		node != VECTOR_EOF; 
		node = vector_next(map->chains[start], node)) {
			MapNode MapN = vector_node_value(map->chains[start], node);
			if (MapN != NULL) {
				if (map->compare(MapN->key, key) == 0) return MapN;
			}
		}
	}

	return MAP_EOF;
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
void map_set_hash_function(Map map, HashFunc func) {
	map->hash_function = func;
}

uint hash_string(Pointer value) {
	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;			// hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
    return hash;
}

uint hash_int(Pointer value) {
	return *(int*)value;
}

uint hash_pointer(Pointer value) {
	return (size_t)value;				// cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}
