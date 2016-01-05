#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include "beargit.h"
#include "util.h"

/* printf/fprintf calls in this tester will NOT go to file. */

#undef printf
#undef fprintf

/* The suite initialization function.
 * You'll probably want to delete any leftover files in .beargit from previous
 * tests, along with the .beargit directory itself.
 *
 * You'll most likely be able to share this across suites.
 */
int init_suite(void)
{
    // preps to run tests by deleting the .beargit directory if it exists
    fs_force_rm_beargit_dir();
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    return 0;
}

/* You can also delete leftover files after a test suite runs, but there's
 * no need to duplicate code between this and init_suite
 */
int clean_suite(void)
{
    return 0;
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void simple_sample_test(void)
{
    // This is a very basic test. Your tests should likely do more than this.
    // We suggest checking the outputs of printfs/fprintfs to both stdout
    // and stderr. To make this convenient for you, the tester replaces
    // printf and fprintf with copies that write data to a file for you
    // to access. To access all output written to stdout, you can read
    // from the "TEST_STDOUT" file. To access all output written to stderr,
    // you can read from the "TEST_STDERR" file.
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
}

struct commit {
  char msg[MSG_SIZE];
  struct commit* next;
};


void free_commit_list(struct commit** commit_list) {
  if (*commit_list) {
    free_commit_list(&((*commit_list)->next));
    free(*commit_list);
  }

  *commit_list = NULL;
}

void run_commit(struct commit** commit_list, const char* msg) {
    int retval = beargit_commit(msg);
    CU_ASSERT(0==retval);

    struct commit* new_commit = (struct commit*)malloc(sizeof(struct commit));
    new_commit->next = *commit_list;
    strcpy(new_commit->msg, msg);
    *commit_list = new_commit;
}

void simple_log_test(void)
{
    struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    FILE* asdf = fopen("asdf.txt", "w");
    fclose(asdf);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!1");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!2");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!3");

    retval = beargit_log(10);
    CU_ASSERT(0==retval);

    struct commit* cur_commit = commit_list;

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    while (cur_commit != NULL) {
      char refline[LINE_SIZE];

      // First line is commit -- don't check the ID.
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

      // Second line is msg
      sprintf(refline, "   %s\n", cur_commit->msg);
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT_STRING_EQUAL(line, refline);

      // Third line is empty
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strcmp(line,"\n"));

      cur_commit = cur_commit->next;
    }

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

    free_commit_list(&commit_list);
}


/**
  * Checks output after comprehensive_merge_test is executed
  */

void comprehensive_merge_test_output(void) {
  FILE* fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  char *actual = "t.txt conflicted copy created\n";
  CU_ASSERT(!strcmp(actual, line));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "x.txt conflicted copy created\n";
  CU_ASSERT(!strcmp(actual, line));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "t.txt conflicted copy created\n";
  CU_ASSERT(!strcmp(actual, line));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "y.txt added\n";
  CU_ASSERT(!strcmp(actual, line));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "t.txt conflicted copy created\n";
  CU_ASSERT(!strcmp(actual, line));

  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "z.txt added\n";
  CU_ASSERT(!strcmp(actual, line));

  fclose(fstdout);
}


/** Comprehensive test of merge
  * Total of 4 branches: master, A,B,C
  * Master contains t.txt, x.txt, and a.txt
  * A contains t.txt and x.txt
  * B contains t.txt and y.txt
  * C contains t.txt and z.txt
  * Merge Order: A into Master,  B Into Master, C into Master.
  * Resulting files in directory: t.txt, x.txt, a.txt, t.txt.<COMMIT_A>,
  * x.txt.<COMMIT_A>, t.txt.<COMMIT_B>, y.txt, t.txt.<COMMIT_C>, z.txt
  * Diagram:
  *    M: init <-- (commit t)  <-- (commit a, x) <----(Merge M/B)
  *    A:             \ <----------(commit x) <-----------/
  *    B:              \ <-------- (commit y)  <---------/
  *    C:               \ <------- (commit z) <---------/

  */
void comprehensive_merge_test(void) {
  int retval;
  retval = beargit_init();
  CU_ASSERT(0 == retval);

  FILE* t = fopen("t.txt", "w");
  fclose(t);
  retval = beargit_add("t.txt");

  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);

  // Creates 3 more branches
  retval = beargit_checkout("A", 1);
  CU_ASSERT(0 == retval);
  retval = beargit_checkout("B", 1);
  CU_ASSERT(0 == retval);
  retval = beargit_checkout("C", 1);
  CU_ASSERT(0 == retval);
  retval = beargit_checkout("master", 0);
  CU_ASSERT(0 == retval);

  //Commit a & x into master
  FILE *x = fopen("x.txt", "w");
  fclose(x);
  FILE *a = fopen("a.txt", "w");
  fclose(a);

  retval = beargit_add("x.txt");
  retval = beargit_add("a.txt");
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);


  //Checkout branch A and commit x.  Capture commit ID
  char commit_a_id[COMMIT_ID_SIZE];
  retval = beargit_checkout("A", 0);
  CU_ASSERT(0 == retval);
  x = fopen("x.txt", "w");
  fclose(x);
  retval = beargit_add("x.txt");
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);
  FILE *prev = fopen(".beargit/.prev", "r");
  fgets(commit_a_id, sizeof(commit_a_id), prev);
  strtok(commit_a_id, "\n");
  fclose(prev);

  //Checkout branch B and commit y.  Capture commit ID
  char commit_b_id[COMMIT_ID_SIZE + FILENAME_SIZE];
  retval = beargit_checkout("B", 0);
  CU_ASSERT(0 == retval);
  FILE *y = fopen("y.txt", "w");
  fclose(y);
  retval = beargit_add("y.txt");
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);
  prev = fopen(".beargit/.prev", "r");
  fgets(commit_b_id, sizeof(commit_b_id), prev);
  strtok(commit_b_id, "\n");
  fclose(prev);

  //Checkout branch C and commit z.  Capture commit ID
  char commit_c_id[COMMIT_ID_SIZE + FILENAME_SIZE];
  retval = beargit_checkout("C", 0);
  CU_ASSERT(0 == retval);
  FILE *z = fopen("z.txt", "w");
  fclose(z);
  retval = beargit_add("z.txt");
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);
  prev = fopen(".beargit/.prev", "r");
  fgets(commit_c_id, sizeof(commit_c_id), prev);
  strtok(commit_c_id, "\n");
  fclose(prev);

  //Checkout to master and merge A,B,C.

  retval = beargit_checkout("master", 0);
  CU_ASSERT(0 == retval);
  retval = beargit_merge("A");
  CU_ASSERT(0 == retval);
  retval = beargit_merge("B");
  CU_ASSERT(0 == retval);
  retval = beargit_merge("C");
  CU_ASSERT(0 == retval);

  //See if files all files are located in current directory
  //with the write commit conflicts

  char filename[FILENAME_SIZE];
  sprintf(filename, "t.txt");
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "x.txt");
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "a.txt");
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "t.txt.%s", commit_a_id);
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "x.txt.%s", commit_a_id);
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "t.txt.%s", commit_b_id);
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "y.txt");
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "t.txt.%s", commit_c_id);
  CU_ASSERT(access(filename, F_OK) != -1);
  sprintf(filename, "z.txt");
  CU_ASSERT(access(filename, F_OK) != -1);
}

/**
  * Checks output after comprehensive_checkout_test is executed
  * for each its operation
  */
void comprehensive_checkout_test_output(void) {
  FILE* fstdout = fopen("TEST_STDOUT", "r");
  CU_ASSERT_PTR_NOT_NULL(fstdout);
  FILE* fstderr = fopen("TEST_STDERR", "r");
  CU_ASSERT_PTR_NOT_NULL(fstderr);
  char *actual;

  const int LINE_SIZE = 512;
  char line[LINE_SIZE];

  //line 540
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  actual = "ERROR:  There are no commits.\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 542
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  actual = "ERROR:  There are no commits.\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 544
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "*  master\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 550
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   master\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "*  br1\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 554
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   master\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br1\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "*  br2\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 558
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  actual = "ERROR:  A branch named master already exists.\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 576
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  actual = "ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 578
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "Tracked files:\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "b.txt\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "There are 1 files total.\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 584
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "Tracked files:\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "There are 0 files total.\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 586
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  actual = "ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 596
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "Tracked files:\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "c.txt\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "There are 1 files total.\n";

  //line 598
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   master\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "*  br1\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br2\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 611
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "Tracked files:\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "There are 0 files total.\n";

  //line 615
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "Tracked files:\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "a.txt\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "There are 1 files total.\n";

  //line 617
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "*  master\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br1\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br2\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 633
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "a.txt added\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));

  //line 651
  actual = "a.txt conflicted copy created\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 644
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
  actual = "ERROR:  No branch or commit br3 exists.\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 684
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "Tracked files:\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "a.txt\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "There are 1 files total.\n";

  //line 686
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   master\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br1\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br2\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   br3\n";
  CU_ASSERT(!strcmp(actual, line));

  //line 688
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "commit cfe062e2c450b32a4fe2c3c3891193ba0e0822fd\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "   THIS IS BEAR TERRITORY!\n";
  CU_ASSERT(!strcmp(actual, line));
  CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
  actual = "\n";

  CU_ASSERT(!strcmp(actual, line));
  fclose(fstdout);
  fclose(fstderr);
}

/** Comprehensive test of checkout
  * Total of 4 branches: master, br1, br2, br3. In the end
  * Master contains a.txt
  * br1 contains a.txt
  * br2 contains 0 files
  * br3 contains a.txt and d.txt and a.txt.<commit_id>
  * Checks if checkout commit_id, checkout -b <new_branch_name>, and checkout <branch_name>
  * work properly and program does not fail.
  */


void comprehensive_checkout_test(void) {
  //initiating beargit and check intial state
  int retval;
  retval = beargit_init();
  CU_ASSERT(0 == retval);
  retval = beargit_log(0);
  CU_ASSERT(1 == retval);
  retval = beargit_log(100);
  CU_ASSERT(1 == retval);
  retval = beargit_branch();
  CU_ASSERT(0 == retval);

  //creating new branches br1 and br2 and checking if they were created
  retval = beargit_checkout("br1", 1);
  CU_ASSERT(0 == retval);
  retval = beargit_branch();
  CU_ASSERT(0 == retval);
  retval = beargit_checkout("br2", 1);
  CU_ASSERT(0 == retval);
  retval = beargit_branch();
  CU_ASSERT(0 == retval);

  //tries to checkout to master bracnh while creating a new branch. Fails. master already exists
  retval = beargit_checkout("master", 1); //created
  CU_ASSERT(1 == retval);

  //creates 3 files: a.txt, b.txt, c.txt while being on different branches. br2 is the last one
  FILE * file_x = fopen("b.txt", "w");
  fclose(file_x);
  retval = beargit_checkout("br1", 0);
  CU_ASSERT(0 == retval);
  FILE * file_y = fopen("a.txt", "w");
  fclose(file_y);
  retval = beargit_checkout("br2", 0);
  CU_ASSERT(0 == retval);
  FILE * file_z = fopen("c.txt", "w");
  fclose(file_z);

  //starts tracking b.txt and to make commit that fails. checks status after
  retval = beargit_add("b.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_commit("TH  IS  IS  B EAR  T E R R I T O R Y!");//gives error
  CU_ASSERT(1== retval);
  retval = beargit_status();//prints status
  CU_ASSERT(0 == retval);
  
  //untracks b.txt. checks the status and tries commit. 2 tries, the first is failing. CHeckout to br1
  retval = beargit_rm("b.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_status();
  CU_ASSERT(0 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITO BEST SQUAD RY!");
  CU_ASSERT(1 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);
  retval = beargit_checkout("br1", 0);
  CU_ASSERT(0 == retval);

  //starts tracking c.txt. checks if we are on the right branch
  retval = beargit_add("c.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_status();
  CU_ASSERT(0 == retval);
  retval = beargit_branch();
  CU_ASSERT(0 == retval);

  //checkout to master. Should delete c.txt from the directory since it is in
  //the current index and not in the index of the master's last commit
  retval = beargit_checkout("master", 0);
  CU_ASSERT(0 == retval);
  char filename[FILENAME_SIZE];
  sprintf(filename, "c.txt");
  CU_ASSERT(access(filename, F_OK) == -1);

  //creating 3 commits while checking functionality of beargit_status, beargit_add, beargit_rm
  //and beargit_branch
  retval = beargit_status();
  CU_ASSERT(0 == retval);
  retval = beargit_add("a.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_status();
  CU_ASSERT(0 == retval);
  retval = beargit_branch();
  CU_ASSERT(0 == retval);
  retval = beargit_commit("fdgdfgdfgdfTHIS IS BEAR TERRITORY!dgddgddggd");
  CU_ASSERT(0 == retval);
  retval = beargit_rm("a.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!dgddgddggd");
  CU_ASSERT(0 == retval);
  retval = beargit_add("a.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!      ");
  CU_ASSERT(0 == retval);

  //checking out to br1 and merging master. commiting
  retval = beargit_checkout("br1", 0);
  CU_ASSERT(0 == retval);
  retval = beargit_merge("master");
  CU_ASSERT(0 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);

  //checks of a.txt is in the directory(checking if merge does not mess up with coping files).
  sprintf(filename, "a.txt");
  CU_ASSERT(access(filename, F_OK) != -1);

  //checking out to br3 without "-b".error
  retval = beargit_checkout("br3", 0); //doesnt exist
  CU_ASSERT(1 == retval);

  //checking out to br3 while creating it.
  retval = beargit_checkout("br3", 1); //creates new branch br3
  CU_ASSERT(0 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);
  retval = beargit_merge("master");
  CU_ASSERT(0 == retval);

  //creating new file on the branch. starts tracking and commits.
  FILE * file_d = fopen("d.txt", "w");
  fclose(file_d);
  retval = beargit_add("d.txt");
  CU_ASSERT(0 == retval);
  retval = beargit_commit("THIS IS BEAR TERRITORY!");
  CU_ASSERT(0 == retval);
  sprintf(filename, "d.txt");
  CU_ASSERT(access(filename, F_OK) != -1);

  //makes sure merging resulted in the conflicted file.
  char id[100];
  FILE *file = fopen(".beargit/.branch_master", "r");
  CU_ASSERT_PTR_NOT_NULL(fgets(id, 100, file));
  fclose(file);
  sprintf(filename, "a.txt.%s", id);
  CU_ASSERT(access(filename, F_OK) != -1);

  //checkout to the exact commit(the exact id was found: since id depends on the previous
  //commit and the branch name, if we know the branch name(br1) and the previous commit("00...0") we can
  //find the id).
  retval = beargit_checkout("cfe062e2c450b32a4fe2c3c3891193ba0e0822fd" , 0);
  CU_ASSERT(0 == retval);

  //file d.txt should not be in the current repository since it was tracked after this commit
  sprintf(filename, "d.txt");
  CU_ASSERT(access(filename, F_OK) == -1);

  //checking status, branch status(none should be current since detached) and checkes if log displays one commit
  CU_ASSERT(0 == retval);
  retval = beargit_status();
  CU_ASSERT(0 == retval);
  retval = beargit_branch();
  CU_ASSERT(0 == retval);
  retval = beargit_log(100);
  CU_ASSERT(0 == retval);
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
   CU_pSuite pSuite = NULL;
   CU_pSuite pSuite2 = NULL;
   CU_pSuite pSuite3 = NULL;
   CU_pSuite pSuite4 = NULL;
   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #1 */
   if (NULL == CU_add_test(pSuite, "Simple Test #1", simple_sample_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   pSuite2 = CU_add_suite("Suite_2", init_suite, clean_suite);
   if (NULL == pSuite2) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the Suite #2 */
   if (NULL == CU_add_test(pSuite2, "Log output test", simple_log_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }



    /* add a suite to the registry */

    /* You don't necessarily have to use the same init and clean functions for
     * each suite. You can specify the function names in the next line:
     */
    pSuite3 = CU_add_suite("Suite_3", init_suite, clean_suite);
    if (NULL == pSuite3) {
       CU_cleanup_registry();
       return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite3, "Comprehensive Merge", comprehensive_merge_test))
    {
       CU_cleanup_registry();
       return CU_get_error();
    }

    if (NULL == CU_add_test(pSuite3, "Comprehensive Merge Output Check", comprehensive_merge_test_output))
    {
       CU_cleanup_registry();
       return CU_get_error();
    }

    //Checkout testing
    pSuite4 = CU_add_suite("Suite_4", init_suite, clean_suite);
    if (NULL == pSuite4) {
       CU_cleanup_registry();
       return CU_get_error();
    }
    /* Add tests to the Suite #4 */
    if (NULL == CU_add_test(pSuite4, "Comprehensive Checkout", comprehensive_checkout_test))
    {
       CU_cleanup_registry();
       return CU_get_error();
    }
    if (NULL == CU_add_test(pSuite4, "Comprehensive Checkout Output", comprehensive_checkout_test_output))
    {
       CU_cleanup_registry();
       return CU_get_error();
    }



   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}