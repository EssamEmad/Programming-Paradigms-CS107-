using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

// you should be implementing these two methods right here... 
bool imdb::getCredits(const string& player, vector<film>& films) const {
    int * numberOfActors = (int* ) actorFile ;
  char const  * playerC = player.c_str();
  void *actor = binarySearch(playerC,0, *numberOfActors,1);
  char *actorChar = (char*) actor;
  if(actorChar == NULL)
    return false;
  // getting the length of the string to move to the actor's data
   int stringLength = 0;
  while( *actorChar != '\0'){
    actorChar ++;
    stringLength ++;
  }
  // if the length of the string is even we add 2 bytes; the null terminator and an aditinoal byte  to the offset to make everything after that be even
  if(stringLength %2 == 0){
    actorChar += 2;
    stringLength += 2;
  } else{
    actorChar ++;
    stringLength ++;
  }
  short *numberOfMovies = (short*) actorChar;
  actorChar +=2;
 // to get the offsets add 2 bytes if the string length + 2 bytes (short storing number of movies) is not a multiple of 4
    if( (stringLength + 2) % 4 != 0)
      actorChar += 2;
  int * movieOffsets = (int*) actorChar;
  //iterate on all the movies and add them to the fimls vector
  for(int i = 0; i< *numberOfMovies; i++){
    film newFilm = getFilm(*movieOffsets);
    films.push_back(newFilm);
    movieOffsets++;
    
  }
 return true;
 }
film imdb::getFilm(const int  pointer)const{
  film returnFilm;
  char * movieName = (char*) movieFile + pointer;
  char * moviePointer = movieName;
  int movieNameLength = 0;
  //move the pointer after the null character
  while(*moviePointer != '\0'){
    moviePointer ++;
    movieNameLength++;
  }
  //reached the end of the name and pointing to the null terminator
  moviePointer++;
  
 returnFilm.year = 1990 + *moviePointer;
  returnFilm.title = string(movieName);
  return returnFilm;
}
  
void *imdb:: getPointerToMovieOffsets(const char * actorChar)const{
  return (void*) actorChar;
}
void* imdb::binarySearch(const char* player,int start, int end, int actor) const 
{
  int * iterator;
  if(actor == 1)
    iterator =(int *) actorFile;
  else
    iterator =(int*) movieFile;
  int * actorPointer = (int *) iterator +1;
  int index = start + (end-start)/2;
 actorPointer += index ;
  char * middleActor = (char*) iterator + *actorPointer;
  int comparingResult = strcmp(player,middleActor);
  //base case
  if(start == end || index < start|| index > end)
    return NULL;
  if(comparingResult == 0)
    return (void*) middleActor;
  else if( comparingResult < 0)
    return binarySearch(player,start,index-1,actor);
  else
    return binarySearch(player, index+1,  end,actor);
  
}
bool imdb::getCast(const film& movie, vector<string>& players) const {
  char * movieNameToSearch =(char*) movie.title.c_str();
  int * numberOfMovies = (int*) movieFile;
  char * movieIterator = (char *) binarySearch(movieNameToSearch, 0, *numberOfMovies,0);
  int length = strlen(movieIterator);
  movieIterator += length;
  movieIterator +=2;
  //if the name + 2 bytes ( 1 for the null terminator and one for the year) is odd we add another null terminator
  if((length +2) % 2 != 0)
    movieIterator++;
  short * numberOfActors = (short *) movieIterator;
  //add 2 bytes representing the numberOfActors casting in the movie
  movieIterator += 2;
  int * offsets = (int *) movieIterator;
 for(int i = 0; i< *numberOfMovies; i++){
   int offset = *offsets++;
   char * playerName = (char*) actorFile + offset;
   players.push_back(string(playerName));
 }
  
 return true;
 }
imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
