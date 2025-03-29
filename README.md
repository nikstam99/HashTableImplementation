![run-tests](../../workflows/run-tests/badge.svg)

##  Simple Project on Hash tables

Η εργασία αυτή αφορά την υλοποίηση και βελτίωση δύο τεχνικών κατακερματισμού: μια υβριδική μέθοδο που συνδυάζει open addressing και separate chaining, και τη μέθοδο Hopscotch hashing. Στην πρώτη άσκηση, τα στοιχεία τοποθετούνται σε γειτονικές θέσεις ή σε vector όταν δεν υπάρχει διαθέσιμη κοντινή θέση. Στη δεύτερη άσκηση, εφαρμόζεται η μέθοδος Hopscotch hashing, όπου προσπαθούμε να τοποθετήσουμε τα στοιχεία κοντά στην κανονική τους θέση, μετακινώντας άλλα στοιχεία αν χρειαστεί. Η υλοποίηση αναλύει τη πολυπλοκότητα των λειτουργιών search και insert και συγκρίνει τις επιδόσεις με την κλασική μέθοδο open addressing.

## Πολυπλοκότητα

Κλασσική Υλοποίηση: 

Search: O(1) average amortized, O(n) worst amortized, O(1) average real, O(n) worst real

Insert: O(1) average amortized, O(n) worst amortized, O(n) average real, O(n) worst real

HybridHash:

Search: O(1) average amortized, O(1) worst amortized, O(1) average real, O(n) worst real

Insert: O(1) average amortized, O(n) worst amortized, O(n) average real, O(n) worst real

HopscotchHash:

Search: O(1) average amortized, O(1) worst amortized, O(1) average real, O(n) worst real

Insert: O(1) average amortized, O(n) worst amortized, O(n) average real, O(n) worst real

