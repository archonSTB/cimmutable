#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include "../debug.h"
#include "./rrb_vect.h"
#include "./rrb_checker.h"

int imc_rrb_size(imc_rrb_t* vec) { 
  if(vec == NULL) return 0;
  if(vec->floor > 0){ 
    return vec->length; 
  }
  int i, count = 0; 
  imc_data_t** data = vec->node.data; 
  for(i = 0; i < ARRAY_SIZE; i++){ 
    if(data[i] != NULL) count++; 
  }
  return count;
} 

imc_rrb_t* imc_rrb_update(imc_rrb_t* vec, int index, imc_data_t* data) {

  /* algorithm */
  puts("\nUpdate in a vector : ");
  imc_rrb_t* new_root;
  new_root = imc_rrb_copy(vec);
  vec = new_root;

  int sub_index;

  while(vec->floor != 1) {
     sub_index = imc_rrb_subindex(vec, index);
     if(vec->node.next[sub_index] != NULL) {
       vec->node.next[sub_index]->refs -= 1;
       //has been up by previous copy, we need to do -1 because we don't reference it in our new copy
       vec->node.next[sub_index] = imc_rrb_copy(vec->node.next[sub_index]);
       // This function updates refs to node
     } else {
       //Should not happen
       //TODO : Free previous
       printf("SHOULD NOT HAPPEN\n");
       return vec;
     }
     printf("Update to the vector of %d elems of floor %d at index %d, taken path : %d\n", vec->length, vec->floor, index, sub_index);
     vec = vec->node.next[sub_index];
  }

  //Here, vec is the first floor
  sub_index = imc_rrb_subindex(vec, index);
  printf("Update to the vector of %d elems of floor %d at index %d, taken path : %d\n", vec->length, vec->floor, index, sub_index);

  if(vec->node.next[sub_index]!=NULL) {
    vec->node.next[sub_index] = imc_rrb_copy_leaf(vec->node.next[sub_index]);
    //This should also copy the data
  } else {
    //Should not happen
    //TODO : Free previous
    return vec;
  }

  vec = vec->node.next[sub_index];
  printf("Update to the vector of %d elems of floor %d at index %d, taken path : %d\n", vec->length, vec->floor, index, sub_index);
  //Here, vec is a fresh leaf with data inside
  //sub_index = imc_rrb_subindex(vec, new_root->length);
  // On veut ajouter à l'indice nb_element
  sub_index = imc_rrb_subindex(vec, index);
  vec->node.data[sub_index] = data;
  printf("Updated to the data at index %d, taken path : %d\n", index, sub_index);

  return new_root;
}

imc_data_t* imc_rrb_lookup(imc_rrb_t* vec, int index) {
  printf("lookup in a vector of size %d at index %d\n", vec->length, index);

/* algorithm */
  int floor_index = 0;
  while(vec->floor!=0) {
    floor_index = imc_rrb_subindex(vec, index);
    if(floor_index != -1) {
      vec = vec->node.next[floor_index];
    } else {
      puts("index cannot be reached.\n");
      return NULL;
    }
  }
  floor_index = imc_rrb_subindex(vec, index);
  return vec->node.data[floor_index];
}

//-->OK
imc_rrb_t* imc_rrb_push_full(imc_rrb_t* vec, imc_data_t* data) {
  puts("\nInsertion in full vector : ");

  /* Make a new root with the vec as first child */
  imc_rrb_t* new_root = imc_rrb_new_root(vec);

  /* Create the path of nodes to add data in the desire leaf */
  new_root->node.next[1] = imc_rrb_create();
  vec = new_root->node.next[1];
  vec->floor = new_root->floor - 1;
  printf("Insertion to the vector of %d elems of floor %d at index %d, taken path : 1\n", new_root->length, new_root->floor, new_root->length);

  while(vec->floor != 0) {
    printf("Insertion to the vector of %d elems of floor %d at index %d, taken path : 0\n", vec->length, vec->floor, new_root->length);
    vec->node.next[0] = imc_rrb_create();
    vec->node.next[0]->floor = vec->floor - 1;
    vec->length = 1;
    vec = vec->node.next[0];
  }
  vec->length = 1;
  vec->node.next[0] = imc_rrb_create_leaf();
  vec = vec->node.next[0];
  printf("Insertion to the vector of %d elems of floor %d at index %d, taken path : 0\n", vec->length, vec->floor, new_root->length);

  /* Add the data to the leaf */
  vec->node.data[0] = data;
  new_root->length += 1;
  printf("Inserted to the data at index %d, taken path : 0\n", new_root->length);

  return new_root;
}

imc_rrb_t* imc_rrb_push_not_full(imc_rrb_t* vec, imc_data_t* data) {
    puts("\nInsertion in not full vector : ");
    /* Make a new root */
    imc_rrb_t* new_root = imc_rrb_copy(vec);

    /* Go through the tree, increasing length and eventually meta by 1
       Until we have reached first floor */
    vec = new_root;
    int insert_index = vec->length;
    int sub_index;
    while(vec->floor != 1) {
       sub_index = imc_rrb_subindex(vec, insert_index);
       if(vec->node.next[sub_index] != NULL) {
         vec->node.next[sub_index]->refs -= 1;
         // has been up by previous copy, we need to do -1
         // because we don't reference it in our new copy
         vec->node.next[sub_index] = imc_rrb_copy(vec->node.next[sub_index]);
         // This function updates refs to node
       } else {
         vec->node.next[sub_index] = imc_rrb_create();
       }
       imc_rrb_increase_length(vec, insert_index);
       printf("Insertion to the vector of %d elems of floor %d at index %d, taken path : %d\n", vec->length, vec->floor, insert_index, sub_index);
       vec = vec->node.next[sub_index];
    }

    /* First floor, we copy the leaf (floor 0) if it exists, or create it */
    sub_index = imc_rrb_subindex(vec, insert_index);
    printf("Insertion to the vector of %d elems of floor %d at index %d, taken path : %d\n", vec->length, vec->floor, insert_index, sub_index);

    if(vec->node.next[sub_index]!=NULL) {
      vec->node.next[sub_index] = imc_rrb_copy_leaf(vec->node.next[sub_index]);
      //This should also copy the data
    } else {
      vec->node.next[sub_index] = imc_rrb_create_leaf();
    }
    imc_rrb_increase_length(vec, insert_index);
    vec = vec->node.next[sub_index];
    printf("Insertion to the vector of %d elems of floor %d at index %d, taken path : %d\n", vec->length, vec->floor, insert_index, sub_index);

    /* We push the new data */
    sub_index = imc_rrb_subindex(vec, insert_index);
    vec->node.data[sub_index] = data;
    imc_rrb_increase_length(vec, insert_index);
    printf("Inserted to the data at index %d, taken path : %d\n", insert_index,
           sub_index);

    return new_root;
}
// add at the end <--- DOXYGENIZE PLEASE!
imc_rrb_t* imc_rrb_push(imc_rrb_t* vec, imc_data_t* data) {

  //algorithm

  if(imc_rrb_full(vec)==1) {
    return imc_rrb_push_full(vec, data);
  } else {
    return imc_rrb_push_not_full(vec, data);
  }
}

imc_rrb_t* imc_rrb_pop(imc_rrb_t* vec, imc_data_t** data) {
  //TODO : Si la branche devient inutile, il faut la supprimer.
  // Idem pour un étage, ça ne coûte pas cher !
  //TODO : Il faut faire les refs.
  //TODO : Il faut faire les free !
  /* preconditions */

  /* invariant */

  /* algorithm */

  // 1) We go to the desire data, copying the path
  // 2) At floor 1, we copy the data node if and only if the looking data
  // is not in first position
  // 3) While copying the data, we remove the last value
  if(vec->length<=0) {
    puts("vector is already empty !\n");
    return vec;
  }
  imc_rrb_t* new_root = imc_rrb_copy(vec);
  vec = new_root;
  int sub_index = 0;
  int last_index = new_root->length-1;
  while(vec->floor!=1) {
    sub_index = imc_rrb_subindex(vec, last_index); // We want the last elem
    vec->node.next[sub_index] = imc_rrb_copy(vec->node.next[sub_index]); // This function updates refs to node
    printf("Pop in a vector of size %d at floor %d, taken : %d \n", vec->length,
           vec->floor, sub_index);
    vec->length -= 1;
    vec = vec->node.next[sub_index];
  }
  //floor 1 : vec's child is a leaf.
  sub_index = imc_rrb_subindex(vec, last_index);
  vec->node.next[sub_index] = imc_rrb_copy_leaf(vec->node.next[sub_index]);
  // This should also copy the data
  printf("Pop in a vector of size %d at floor %d, taken : %d \n", vec->length,
         vec->floor, sub_index);
  vec->length -= 1;
  vec = vec->node.next[sub_index];
  //floor 0 : vec is a leaf
  sub_index = imc_rrb_subindex(vec, last_index);
  printf("Extracted data in a vector of size %d at floor %d, taken : %d \n",
         vec->length, vec->floor, sub_index);
  *data = vec->node.data[sub_index];
  vec->node.data[sub_index] = NULL;
  vec->length -= 1;
  return new_root;

}

int imc_rrb_split(imc_rrb_t* vec_in, int index, imc_rrb_t** vec_out1,
                  imc_rrb_t** vec_out2) {
  imc_rrb_build_left(vec_in, *vec_out1, index);
  imc_rrb_build_right(vec_in, *vec_out2, index);
  return 0;
}

void imc_rrb_build_left(imc_rrb_t* vec_in, imc_rrb_t* left,
                        int index){
  int i, split_index = imc_rrb_subindex(vec_in, index);

  left->floor = vec_in->floor;
  if(vec_in->floor == 0){
    for(i = 0; i <= split_index; i++){
      left->node.data[i] = vec_in->node.data[i];
    }
    left->length = split_index+1;
    return;
  }
  int is_balanced = imc_rrb_balanced(vec_in);
  if(is_balanced == 0) left->meta = malloc(sizeof(int) * ARRAY_SIZE);
  
  for(i = 0; i < split_index; i++){
    left->node.next[i] = vec_in->node.next[i];
    left->node.next[i]->refs++;
    if(is_balanced == 0) left->meta[i] = vec_in->meta[i];
  }
  if(vec_in->floor > 1) left->node.next[split_index] = imc_rrb_create();
  else left->node.next[split_index] = imc_rrb_create_leaf();
 
  imc_rrb_build_left(vec_in->node.next[split_index],
                     left->node.next[split_index], index);
  if(is_balanced == 0){
    if(split_index == 0){
      left->meta[split_index] = imc_rrb_size(left->node.next[split_index]);
    }
    else{
      left->meta[split_index] =
        imc_rrb_size(left->node.next[split_index]) + left->meta[split_index-1];
    }
    left->length = left->meta[split_index];
  }
  else{
    left->length = vec_in->length
      - (imc_rrb_size(vec_in->node.next[split_index])
         - imc_rrb_size(left->node.next[split_index]));
  }
}

void imc_rrb_build_right(imc_rrb_t* vec_in, imc_rrb_t* right,
                        int index){
  int i, j, split_index = imc_rrb_subindex(vec_in, index);
  right->floor = vec_in->floor;
  if(vec_in->floor == 0){
    if(split_index == ARRAY_SIZE-1){
      right->length = -1;
      return;
    }
    for(j = 0, i = split_index+1; i > ARRAY_SIZE; i++, j++){
      right->node.data[j] = vec_in->node.data[i];
    }
    right->length = j+1;
    return;
  }
  right->meta = malloc(sizeof(int) * ARRAY_SIZE);
  
  for(i = split_index+1; i < ARRAY_SIZE; i++){
    right->node.next[i] = vec_in->node.next[i];
    right->node.next[i]->refs++;
  }
  if(vec_in->floor > 1) right->node.next[split_index] = imc_rrb_create();
  else right->node.next[split_index] = imc_rrb_create_leaf();

  imc_rrb_build_right(vec_in->node.next[split_index],
                      right->node.next[split_index], index);

  if(right->node.next[split_index]->length == -1){
    imc_rrb_unref(right->node.next[split_index]);
    right->node.next[split_index] = NULL;
    int all_null = right->node.next[0] == NULL;
    for(i = 1; i < ARRAY_SIZE; i++){
      all_null = all_null && right->node.next[i] == NULL;
      if(!all_null) break;
    }
    if(all_null){
      right->length = -1;
      return;
    }
  }
  for(i = 0; i < ARRAY_SIZE; i++){
    if(i < split_index) right->meta[i] = 0;
    if(i == split_index){
      right->meta[i] = imc_rrb_size(right->node.next[split_index]);
    }
    if(i > split_index){
      right->meta[i] = right->meta[i-1] + imc_rrb_size(right->node.next[i]); 
    }
  }
  right->length = right->meta[ARRAY_SIZE-1]; 
}

imc_rrb_t* imc_rrb_merge(imc_rrb_t* vec_front, imc_rrb_t* vec_tail) {

  return NULL;
}

/* user-side memory management */

int imc_rrb_unref(imc_rrb_t* vec) {
  int i, nb_refs = vec->refs-1;
  vec->refs = nb_refs;
  if(nb_refs == 0){
    if(vec->floor == 0){
      /*TODO eventually : error checking*/
      free(vec->node.data);
      free(vec);
      return 1;
    }
    imc_rrb_t** next = vec->node.next;
    for(i = 0; i < ARRAY_SIZE; i++){
      if(next[i] != NULL) imc_rrb_unref(next[i]);
    }
    free(vec->node.next);
    free(vec);
    return 1;
  }
  return 0;
}

void imc_rrb_dump(imc_rrb_t* vec) {

  return;
}

/* utils */
void imc_rrb_emit(imc_rrb_t* vec, const char* path, char* (*print)(imc_data_t*)) {
  FILE *fp;
  fp = fopen(path, "w+");
  //initialisation :
  fprintf(fp, "digraph g {\n");
  fprintf(fp, "node [shape = record,height=.1];\n");
  emit_node(vec, NULL, "", fp, print);
  fprintf(fp, "}\n");
  fclose(fp);
}

//On pourrait faire un emit_node_compact ?
void emit_node(imc_rrb_t* vec, char* from, char* prefix, FILE* fp,
               char* (*print)(imc_data_t*)) {
  /* added i (as identifier) to the prefix, because of a graphviz bug :
   * https://rt.cpan.org/Public/Bug/Display.html?id=105171
   * Please use after that dot -Tsvg INPUT.dot -o OUTPUT.svg
   * and use a good visualizer if your graph is large.
   */

  fprintf(fp, "node_%s[label = \"", prefix);
  //Pas exactement pareil si c'est une feuille
  char suffix;
  if(vec->floor != 0) {
    for(int i =0; i<ARRAY_SIZE; i++) {
      if(vec->node.next[i] != NULL) {
        suffix = i<10?i+48:i+55;
        //printf("prefix : %s\n", prefix);
        //printf("suffix : %c\n", suffix);
        fprintf(fp, "<i%s> %d", concatc(prefix, suffix), i);
        if(i != ARRAY_SIZE-1 && vec->node.next[i+1] != NULL) {
          fprintf(fp, "| ");
        }
      }
    }
    fprintf(fp, "\"];\n");

    if(from != NULL) {
      fprintf(fp, "%s -> \"node_%s\":i%s0;\n", from, prefix, prefix);
    }
    //"node0":f2 -> "node4":f1;
    for(int i =0; i<ARRAY_SIZE; i++) {
      if(vec->node.next[i] != NULL) {
        suffix = i<10?i+48:i+55;
        char* str_from = malloc(sizeof(char) * 100);
        sprintf(str_from, "\"node_%s\":i%s", prefix, concatc(prefix, suffix));
        emit_node(vec->node.next[i], str_from,
                  concatc(prefix, suffix), fp, print);
      }
    }
  } else {
      for(int i =0; i<ARRAY_SIZE; i++) {
        if(vec->node.data[i] != NULL) {
          suffix = i<10?i+48:i+55;
          fprintf(fp, "<i%s> %s ", concatc(prefix, suffix),
                  print(vec->node.data[i]));
          if(i != ARRAY_SIZE-1 && vec->node.next[i+1] != NULL) {
            fprintf(fp, "|");
          }
        }
      }
      fprintf(fp, "\"];\n");
      if(from != NULL) {
        fprintf(fp, "%s -> \"node_%s\":i%s0;\n", from, prefix, prefix);
      }
  }
}

char* concatc(char* str, char c) {
    size_t len = strlen(str);
    char *str2 = malloc(len + 1 + 1 );
    strcpy(str2, str);
    str2[len] = c;
    str2[len + 1] = '\0';
    return str2;
}

/* return 1 if the vector is full */
int imc_rrb_full(imc_rrb_t* vec) {
  if(imc_rrb_balanced(vec) == 1) {
    return (vec->length == (32*pow(ARRAY_SIZE, vec->floor))) ? 1 : 0;
  } else {
    while(vec->floor > 1) {
      if(vec->meta[ARRAY_SIZE-1]==0) {
        return 0;
      }
    }
    vec = vec->node.next[ARRAY_SIZE-1];
    return (vec->node.data[ARRAY_SIZE-1]!=NULL) ? 1 : 0;
  }
}

/* Add a new root to the top of a vector */
imc_rrb_t* imc_rrb_new_root(imc_rrb_t* vec) {
  /* Create a new root */
  imc_rrb_t* new_root = imc_rrb_create();
  new_root->length = vec->length;
  new_root->floor = vec->floor+1;
  /* vec is his first child */
  new_root->node.next[0] = vec;
  new_root->node.next[0]->refs+=1;

  return new_root;
}

/* return 1 if the vector is balanced */
int imc_rrb_balanced(imc_rrb_t* vec) {
  /* /!\ Is that always true ? What if the rrb has been rebalanced ? /!\ */
  return (vec->meta==NULL) ? 1 : 0;
}

void imc_rrb_increase_length(imc_rrb_t* vec, int index) {
  vec->length+=1;
  /* If the node is balanced, or if it is a leaf, meta is null */
  if(vec->meta != NULL) {
    for(int i = 0; i<ARRAY_SIZE; i++) {
      if(index <= vec->meta[i]) {
        vec->meta[i] += 1;
        if(i != ARRAY_SIZE-1) {
          vec->meta[ARRAY_SIZE-1] += 1;
        }
        break;
      }
    }
  }
}

/* return the subindex, i.e. the subindex you may choose at your current
   floor to go to the vector index */
int imc_rrb_subindex(imc_rrb_t* vec, int index) {
  int floor_index = 0;
  if(imc_rrb_balanced(vec)==1) {
//Si on est balanced, on sait tout de suite où aller
    floor_index = (index >> (int)(log2((double)ARRAY_SIZE) * vec->floor))
      & (ARRAY_SIZE-1);
  } else { //Si on est unbalanced, on doit chercher où aller
    while(vec->meta[floor_index] <= index) {
      floor_index++;
      if(floor_index==ARRAY_SIZE) { //We didn't find it
        return -1;
      }
    }
  }
  return floor_index;
}

imc_rrb_t* imc_rrb_copy_leaf(imc_rrb_t* vec) {
  imc_rrb_t* vec_copy = imc_rrb_create_leaf();
  vec_copy -> floor = vec -> floor;
  vec_copy -> length =  vec -> length;
  vec_copy -> meta = vec -> meta;
  // A changer en allouant un nouveau tableau meta
  if(vec -> node.data != NULL) {
    vec_copy -> node.data = malloc(sizeof(imc_data_t*) * ARRAY_SIZE);
    for(int i = 0; i < ARRAY_SIZE ; i++) {
      vec_copy -> node.data[i] = vec -> node.data[i];
    }
  }
  return vec_copy;
}

imc_rrb_t* imc_rrb_copy(imc_rrb_t* vec) {
  imc_rrb_t* vec_copy = imc_rrb_create();
  vec_copy -> floor = vec -> floor;
  vec_copy -> length =  vec -> length;
  vec_copy -> meta = vec -> meta;
  // A changer en allouant un nouveau tableau meta
  vec_copy -> refs = 1; //It is a copy, so we have one and only one ref to it
  if(vec -> node.next != NULL) {
    vec_copy -> node.next = malloc(sizeof(imc_rrb_t*) * ARRAY_SIZE);
    for(int i = 0; i < ARRAY_SIZE ; i++) {
      vec_copy -> node.next[i] = vec -> node.next[i];
      if(vec_copy -> node.next[i] != NULL) {
        vec_copy -> node.next[i] -> refs += 1;
      }
    }
  }
  return vec_copy;
}

imc_rrb_t* imc_rrb_create_leaf() {
  imc_rrb_t* vec = malloc(sizeof(imc_rrb_t));

  if(vec == NULL){
      LOG(LOG_FATAL, "Allocation failure %s", strerror(errno));
      return NULL;
  }
  vec -> floor = 0;
  vec -> refs = 1;
  vec -> length = 0;
  vec -> meta = NULL;
  vec -> node.data = malloc(sizeof(imc_data_t*) * ARRAY_SIZE);

  if(vec -> node.data == NULL){
      LOG(1, "Allocation failure %s", strerror(errno));
      free(vec);
      return NULL;
  }

  for(int i = 0; i<ARRAY_SIZE; i++) {
    vec -> node.data[i] = NULL;
  }
  return vec;
}

/* --> rrb_vect.h */
/* TODO -> invariant , pre/post cond */
imc_rrb_t* imc_rrb_create() {
    imc_rrb_t* vec = malloc(sizeof(imc_rrb_t));

    if(vec == NULL){
        LOG(LOG_FATAL, "Allocation failure %s", strerror(errno));
        puts("Erreur allocation ! \n");
        return NULL;
    }
    vec -> floor = 1;
    vec -> refs = 1;
    vec -> length = 0;
    vec -> meta = NULL;
    vec -> node.next = malloc(sizeof(imc_rrb_t*) * ARRAY_SIZE);

    if(vec -> node.next == NULL){
        LOG(1, "Allocation failure %s", strerror(errno));
        free(vec);
        return NULL;
    }
    for(int i = 0; i<ARRAY_SIZE; i++) {
      vec-> node.next[i] = NULL;
    }

    return vec;
}
