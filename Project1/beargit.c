#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"
#define  BEARGIT_LENGTH 9
/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - Here are some of the helper functions from util.h:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the project spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR:  File <filename> has already been added.
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR:  File %s has already been added.\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the project spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  fprintf(stdout,"Tracked files:\n\n");
  int count = 0;
  FILE *index = fopen(".beargit/.index", "r");
  char file_name[FILENAME_SIZE];
  while (fgets(file_name, sizeof(file_name),index)) {
//    strtok(file_name, "\n");
    fprintf(stdout,"%s", file_name);
    ++count;
  }

  fprintf(stdout, "\n");
  fprintf(stdout, "There are %d files total.\n", count);
  fclose(index);

  return 0;
}

/* beargit rm <filename>
 *
 * See "Step 2" in the project spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE *findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  int exist = 0;
  char line[FILENAME_SIZE];

  while (fgets(line, sizeof(line),findex)) {
    strtok(line, "\n");
    if(strcmp(line, filename) == 0){
      exist = 1;
      continue;
    }
    fprintf(fnewindex, "%s\n", line);
  }

  if (exist == 0) {
    fprintf(stdout, "ERROR:  File %s not tracked.\n", filename);
    fclose(findex);
    fclose(fnewindex);
    fs_rm(".beargit/.newindex");
    return 1;
  }
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");
  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the project spec.
 *
 */

const char* go_bears = "THIS IS BEAR TERRITORY!";

int check_equal(const char *pos_mes){
  const char *tibt = go_bears;
  while(*pos_mes++ == *tibt++){
    if(*tibt == '\0')
      return 1;
  }
  return 0;
}

int is_commit_msg_ok(const char* msg) {
  /* COMPLETE THE REST */
  while(msg != NULL && *msg != '\0'){
    if(*msg == 'T') {
      if(check_equal(msg))
        return 1;
    }
    ++msg;
  }
  return 0;
}

/* Use next_commit_id to fill in the rest of the commit ID.
 *
 * Hints:
 * You will need a destination string buffer to hold your next_commit_id, before you copy it back to commit_id
 * You will need to use a function we have provided for you.
 */

void next_commit_id(char* commit_id) {
   /* COMPLETE THE REST */
  char cur_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", cur_branch, BRANCHNAME_SIZE);
  if(!strlen(cur_branch)) {
    *commit_id = '\0';
  } else {
    char combined[BRANCHNAME_SIZE + COMMIT_ID_SIZE];
    sprintf(combined, "%s%s", cur_branch, commit_id);
    //fprintf(stdout, "Crypto %s \n", commit_id);
   // fprintf(stdout, "%s\n", cur_branch);
    cryptohash(combined, commit_id);
   // fprintf(stdout, "Crypto-After %s \n", commit_id);
  }
}


void copy_files(char *file_location, char *final_destination, int is_hidden) {
    char destination_of_file[BEARGIT_LENGTH + COMMIT_ID_SIZE + FILENAME_SIZE];
    if (is_hidden) {
        char *helper_pointer = file_location;
        helper_pointer += BEARGIT_LENGTH;
        sprintf(destination_of_file, "%s/%s", final_destination,helper_pointer);
       // fprintf(stdout, "%s\n", destination_of_file);
    } else {
        sprintf(destination_of_file, "%s/%s", final_destination,file_location);
    }
    fs_cp(file_location, destination_of_file);

}

int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);
  //fprintf(stdout, "Before Kostya's mistake");

  /* COMPLETE THE REST */
  if(*commit_id == '\0'){
    fprintf(stderr, "ERROR:  Need to be on HEAD of a branch to commit.\n");
    return 1;
  }
  char bear_directory[BEARGIT_LENGTH + COMMIT_ID_SIZE];
  char fileNames[FILENAME_SIZE];
  sprintf(bear_directory, ".beargit/%s", commit_id);
  fs_mkdir(bear_directory);
  copy_files(".beargit/.index", bear_directory, 1);
  copy_files(".beargit/.prev", bear_directory, 1);

  FILE *tracked_files = fopen(".beargit/.index", "r");

  while(fgets(fileNames,sizeof(fileNames), tracked_files)) {
      strtok(fileNames, "\n");
      copy_files(fileNames, bear_directory, 0);
  }
  sprintf(fileNames,"%s/.msg", bear_directory);
  write_string_to_file(fileNames,msg);

  write_string_to_file(".beargit/.prev", commit_id);
  fclose(tracked_files);
  return 0;
}

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int extract_message_and_get_prev(char *curr_id) {
    //fprintf(stdout,"In extract message\n");
    char directory_navigator[BEARGIT_LENGTH + COMMIT_ID_SIZE + 6];
    sprintf(directory_navigator, ".beargit/%s/.msg", curr_id);
    if (access(directory_navigator, F_OK) == -1){
       // fprintf(stdout, "in if\n");
        return 1;
    }

    fprintf(stdout, "commit %s\n", curr_id);
    char message[MSG_SIZE];
    read_string_from_file(directory_navigator, message, sizeof(message));
    fprintf(stdout,"   %s", message);
    fprintf(stdout,"\n\n");

    sprintf(directory_navigator, ".beargit/%s/.prev", curr_id);
    //fprintf(stdout, "*********BEFORE-SWITCH ************\n%s\n", curr_id);
    read_string_from_file(directory_navigator,curr_id, COMMIT_ID_SIZE);
    //fprintf(stdout,"*********AFTER-SWITCH ************\n%s\n", curr_id);



 return 0;

}

int initialCheck() {
    char prev_id[COMMIT_ID_SIZE];
    read_string_from_file(".beargit/.prev",prev_id, sizeof(prev_id));
    return strcmp(prev_id, "0000000000000000000000000000000000000000");
}

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  int counter = 0;
  if (!initialCheck()) {
      fprintf(stderr, "ERROR:  There are no commits.\n");
      return 1;
  } else {
      char prev_commit_id[COMMIT_ID_SIZE];
    //fprintf(stdout, "Before while\n");
      read_string_from_file(".beargit/.prev",prev_commit_id, sizeof(prev_commit_id));
      while ((counter < limit) && !extract_message_and_get_prev(prev_commit_id)) {
          //fprintf(stdout,"In While\n");
          ++counter;
      }


  }
  return 0;
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

int beargit_branch() {
  /* COMPLETE THE REST */
  FILE *branch_file = fopen(".beargit/.branches", "r");
  char branch[BRANCHNAME_SIZE];
  char curr_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", curr_branch, sizeof(curr_branch));
  //fprintf(stdout,"%s\n", curr_branch);
  while (fgets(branch, sizeof(branch), branch_file)) {
    strtok(branch, "\n");
    //fprintf(stdout, "%s\n", branch);
    if ( !strcmp(branch,curr_branch)) {
        fprintf(stdout, "*  %s\n", branch);
    } else {
        fprintf(stdout, "   %s\n", branch);
    }
  }
  fclose(branch_file);

  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

void delete_tracked_files() {
  FILE *index = fopen(".beargit/.index", "r");
  char tracked_file[FILENAME_SIZE];
  while (fgets(tracked_file, FILENAME_SIZE, index)) {
    strtok(tracked_file, "\n");
    if (access(tracked_file, F_OK) != -1) {
      fs_rm(tracked_file);
    }
  }
  fclose(index);
}

void load_files_from_commit(const char *commit_id) {
  char commit_index[BEARGIT_LENGTH + COMMIT_ID_SIZE + 7];
  sprintf(commit_index, ".beargit/%s/.index", commit_id);
  char file_name[FILENAME_SIZE];
  FILE *index = fopen(commit_index, "r");
  char file_name_in_commit[BEARGIT_LENGTH + COMMIT_ID_SIZE + FILENAME_SIZE + 1];
  while (fgets(file_name, sizeof(file_name), index)) {
    strtok(file_name, "\n");
    sprintf(file_name_in_commit, ".beargit/%s/%s", commit_id, file_name);
    fs_cp(file_name_in_commit, file_name);
  }
  fclose(index);
}


int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  if (!strcmp(commit_id, "0000000000000000000000000000000000000000")) {
    delete_tracked_files();
    //write_string_to_file(".beargit/.index", "");
    FILE *index_new = fopen(".beargit/.index", "w");
    fclose(index_new);
    write_string_to_file(".beargit/.prev", commit_id);
    return 0;

  }

  delete_tracked_files();
  char path[BEARGIT_LENGTH + COMMIT_ID_SIZE + 7];
  sprintf(path, ".beargit/%s/.index", commit_id);
  load_files_from_commit(commit_id);
  fs_cp(path, ".beargit/.index");
  // sprintf(path, ".beargit/%s/.prev", commit_id);
  // fs_cp(commit_id, ".beargit/.prev");
  write_string_to_file(".beargit/.prev", commit_id);



  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  char directory[BEARGIT_LENGTH + COMMIT_ID_SIZE];
  sprintf(directory, ".beargit/%s", commit_id);

  if (access(directory, F_OK) == -1) {
    return 0;
  }
  return 1;

}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, leave the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

   // Check whether the argument is a commit ID. If yes, we just change to detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    // ...and setting the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }



  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(arg) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", arg);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
    return 1;
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // File for the branch we are changing into.
  char branch_file[BRANCHNAME_SIZE + 50] = ".beargit/.branch_";
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file);
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}

/* beargit reset
 *
 * See "Step 7" in the project spec.
 *
 */

int beargit_reset(const char* commit_id, const char* filename) {
  // Check if the file is in the commit directory
  /* COMPLETE THIS PART */

  // Copy the file to the current working directory
  /* COMPLETE THIS PART */

  // Add the file if it wasn't already there
  /* COMPLETE THIS PART */
  if (!is_it_a_commit_id(commit_id)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", commit_id);
      return 1;
  }
  char restore_file[BEARGIT_LENGTH + COMMIT_ID_SIZE + FILENAME_SIZE +1];
  sprintf(restore_file, ".beargit/%s/%s", commit_id, filename);
  if(access(restore_file, F_OK) == -1){
    fprintf(stderr, "ERROR:  %s is not in the index of commit %s.\n", filename, commit_id);
    return 1;
  }
  fs_cp(restore_file, filename);
  char file[FILENAME_SIZE];
  FILE* index = fopen(".beargit/.index", "a+");
  while(fgets(file, sizeof(file), index)){
    strtok(file,"\n");
    if(!strcmp(file, filename)) {
      fclose(index);
      return 0;
    }
  }

  fprintf(index, "%s\n",filename);
  fclose(index);

  return 0;
}

/* beargit merge
 *
 * See "Step 8" in the project spec.
 *
 */
int file_in_index(char * filename) {
  FILE *index = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), index)){
    strtok(line, "\n");
    if(!strcmp(line, filename)){
      fclose(index);
      return 1;
    }
  }
  fclose(index);
  index = fopen(".beargit/.index", "a");
  fprintf(index, "%s\n", filename);
  fclose(index);
  return 0;
}

int beargit_merge(const char* arg) {
  // Get the commit_id or throw an error
  char commit_id[COMMIT_ID_SIZE];
  if (!is_it_a_commit_id(arg)) {
      if (get_branch_number(arg) == -1) {
            fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
            return 1;
      }
      char branch_file[FILENAME_SIZE];
      snprintf(branch_file, FILENAME_SIZE, ".beargit/.branch_%s", arg);
      read_string_from_file(branch_file, commit_id, COMMIT_ID_SIZE);
  } else {
      snprintf(commit_id, COMMIT_ID_SIZE, "%s", arg);
  }

  // Iterate through each line of the commit_id index and determine how you
  // should copy the index file over
   /* COMPLETE THE REST */
  char commit_index[BEARGIT_LENGTH + COMMIT_ID_SIZE + 10];
  sprintf(commit_index,".beargit/%s/.index", commit_id);
  FILE *index = fopen(commit_index, "r");
  char file[FILENAME_SIZE];
  while(fgets(file, sizeof(file), index)) {
    strtok(file, "\n");
    if(file_in_index(file)){
      char confilcted_file[FILENAME_SIZE +COMMIT_ID_SIZE+10];
      char file_name_in_commit[BEARGIT_LENGTH + COMMIT_ID_SIZE + FILENAME_SIZE+10];
      sprintf(file_name_in_commit, ".beargit/%s/%s", commit_id, file);
      sprintf(confilcted_file, "%s.%s",file,commit_id);
      fs_cp(file_name_in_commit, confilcted_file);
      fprintf(stdout, "%s conflicted copy created\n", file);
    } else {
      char file_name_in_commit[BEARGIT_LENGTH + COMMIT_ID_SIZE + FILENAME_SIZE+10];
      sprintf(file_name_in_commit, ".beargit/%s/%s", commit_id, file);
      fs_cp(file_name_in_commit, file);
      fprintf(stdout, "%s added\n", file);
    }
  }
  fclose(index);
  return 0;
}
