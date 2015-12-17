#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef enum {
  Integer, String, List, Nil
} nodeType;
char *ConcatAll(nodeType *list);

int main(){

}

char * ConcatAll(nodeType * list, char* concatString){
  //base case
  if(*list == Integer || *list == Nil)
    return concatString;
  if(*list == String){
    //concatinate this string to the concatString
    char * nodeStr = (char*) list + sizeof(nodeType);
    int totalLength = strlen(concatString) + strlen(nodeStr) +1;// extra character for null terminator
    char * newStr =(char*) malloc(totalLength);
    strcpy(newStr, concatString);
    strcpy(newStr + strlen(concatString) , nodeStr);
    free(concatString);
    free(nodeStr);
    return newStr;
  } else{
    //this node is a list
    nodeType * firstNode = (nodeType *)((char*) list + sizeof(nodeType));
    nodeType * secondNode = (nodeType *) ( (char*)firstNode + sizeof(nodeType));
    ConcatAll(firstNode, concatString);
    ConcatAll(secondNode, concatString);
    return concatString;
  }

  
}
