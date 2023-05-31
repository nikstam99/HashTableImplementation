/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω υβριδικού Hash Table
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "ADTVector.h"
#include "ADTMap.h"

// Οι κόμβοι του map στην υλοποίηση με hash table, μπορούν να είναι σε 2 διαφορετικές καταστάσεις,
// κενοί και μη κενοί
typedef enum {
	EMPTY, OCCUPIED, VECTOR
} State;

// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Κάθε θέση i θεωρείται γεινοτική με όλες τις θέσεις μέχρι και την i + NEIGHBOURS
#define NEIGHBOURS 3


// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;		// Το κλειδί που χρησιμοποιείται για να hash-αρουμε
	Pointer value;  	// Η τιμή που αντισοιχίζεται στο παραπάνω κλειδί
	State state;	// Μεταβλητή για να μαρκάρουμε την κατάσταση των κόμβων 
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	MapNode array;				// Ο πίνακας που θα χρησιμοποιήσουμε για το map (remember, φτιάχνουμε ένα hash table)
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

	// Αρχικοποιούμε τους κόμβους που έχουμε σαν διαθέσιμους.
	for (int i = 0; i < map->capacity; i++)
		map->array[i].state = EMPTY;

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

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value) {
	// Σκανάρουμε το Hash Table μέχρι να βρούμε διαθέσιμη θέση για να τοποθετήσουμε το ζευγάρι,
	// ή μέχρι να βρούμε το κλειδί ώστε να το αντικαταστήσουμε.
	bool already_in_map = false;
	Pointer found_v = NULL;
	MapNode node = NULL;
	uint pos;
	uint stop = (map->hash_function(key) % map->capacity + NEIGHBOURS) % map->capacity;
	uint start = map->hash_function(key) % map->capacity;
	for (pos = start;										// ξεκινώντας από τη θέση που κάνει hash το key
		pos != stop;										// αν φτάσουμε στο stop σταματάμε
		pos = (pos + 1) % map->capacity) {					// linear probing, γυρνώντας στην αρχή όταν φτάσουμε στη τέλος του πίνακα

		if (map->array[pos].key == NULL) break;

		if (map->compare(map->array[pos].key, key) == 0) {
			already_in_map = true;
			node = &map->array[pos];						// βρήκαμε το key, το ζευγάρι θα μπει αναγκαστικά εδώ 
			break;											// και δε χρειάζεται να συνεχίζουμε την αναζήτηση.
		}
	}
	
	if (map->array[pos].key == NULL)						// αν βρήκαμε EMPTY, το node δεν έχει πάρει ακόμα τιμή
	node = &map->array[pos];
	else if (node == NULL && pos == stop)
	node = &map->array[start];

	if (pos == stop && map->array[pos].key != NULL && node->state == VECTOR) { 		// Αν φτάσαμε στο pos + NEIGHBOURS ψάχνουμε στο vector (αν υπάρχει) στην θέση 
		found_v = vector_find(map->array[start].value, value, map->compare);		// που κάνουμε το hash
		if (found_v != NULL) {
			already_in_map = true;
		}
	}

	if (already_in_map == false && pos == stop && map->array[pos].key != NULL) {
		if (map->array[start].state == OCCUPIED) {
			Pointer previous_v = node->value;
			Pointer previous_k = node->key;
			node->value = vector_create(0, free);
			node->key = vector_create(0, free);
			vector_insert_last(node->value, previous_v);
			vector_insert_last(node->key, previous_k);
			node->state = VECTOR;
		}
		vector_insert_last(node->value, value);
		vector_insert_last(node->key, key);
		map->size++;
		return;
	}
	// Σε αυτό το σημείο, το node είναι ο κόμβος στον οποίο θα γίνει εισαγωγή.
	if (already_in_map) {
		if (node->state == OCCUPIED) {
			// Αν αντικαθιστούμε παλιά key/value, τa κάνουμε destropy
			if (node->key != key && map->destroy_key != NULL)
				map->destroy_key(node->key);

			if (node->value != value && map->destroy_value != NULL)
				map->destroy_value(node->value);
		}
		else if (node->state == VECTOR) {
			Pointer vector_last_value = vector_node_value(node->value, vector_last(node->value));
			Pointer vector_last_key = vector_node_value(node->key, vector_last(node->key));
			int i = 0;
			for (VectorNode v_node = vector_first(node->value); 
				v_node != vector_last(node->value); 
				v_node = vector_next(node->value, v_node)) {

					if (vector_node_value(node->value, v_node) == found_v) break;
					else i++;
			}
			vector_set_at(node->value, i, vector_last_value);
			vector_set_at(node->key, i, vector_last_key);
			vector_remove_last(node->value);
			vector_remove_last(node->key);
		}
	}
	// Προσθήκη τιμών στον κόμβο
	node->state = OCCUPIED;
	node->key = key;
	node->value = value;
	map->size++;
}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	return false;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.
Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);

	bool found = false;
	int i;
	if (node->state == VECTOR) {
		Vector vec = node->key;
		for (i = 0; i < vector_size(vec); i++) {
			if (map->compare(vector_get_at(vec, i), key) == 0) {
				found = true;
				break;
			}
		}
		if (found) return vector_get_at(vec, i);
		else return NULL;
	}
	else if (node != MAP_EOF)
		return node->value;
	else
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	return NULL;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	return NULL;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {

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
	for (int i = node - map->array + 1; i < map->capacity; i++)
		if (map->array[i].state == OCCUPIED)
			return &map->array[i];

	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node) {
	if (node->state == VECTOR)
	return NULL;
	else return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	if (node->state == VECTOR)
	return NULL;
	else return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	uint pos;
	uint stop = (map->hash_function(key) % map->capacity + NEIGHBOURS) % map->capacity;
	uint start = map->hash_function(key) % map->capacity;
	for (pos = start;										// ξεκινώντας από τη θέση που κάνει hash το key
		pos != stop;										// αν φτάσουμε στο stop σταματάμε
		pos = (pos + 1) % map->capacity) {					// linear probing, γυρνώντας στην αρχή όταν φτάσουμε στη τέλος του πίνακα

		// Μόνο σε OCCUPIED θέσεις, ελέγχουμε αν το key είναι εδώ
		if (map->array[pos].state == OCCUPIED && map->compare(map->array[pos].key, key) == 0)
		return &map->array[pos];
	}
	if (map->array[start].state == VECTOR) {
		int i = 0;
		Vector vec = map->array[start].key;
		for (VectorNode v_node = vector_first(vec); 
			v_node != vector_last(vec); 
			v_node = vector_next(vec, v_node)) {
				if (map->compare(vector_get_at(vec, i), key) == 0) return &map->array[start];
				i++;
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