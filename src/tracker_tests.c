/*
  Tracker Tests
  Daniel Bishop

  Unit Tests for tracker_serv.c and tracker.c
*/

#include <assert.h>

#include "tracker.h"

void testBuildMsg();
void testAddIfAbsent();

int main() {
  testBuildMsg();
  testAddIfAbsent();
  printf("All tests ran successfully.\n");
}

/*
  Ensure that the buildMsg() method correctly prepends a string message with the
  length of the string.
*/
void testBuildMsg() {
  char *message = "test message please ignore";
  char *built_msg = buildMsg(message);
  assert(built_msg[0] == strlen(message));
  assert(strncmp(built_msg + 1, message, strlen(message)) == 0);
}

/*
  Test that the linked list implementation correctly adds new clients and does
  not add duplicate entries.
*/
void testAddIfAbsent() {
  Client *head = malloc(sizeof(Client));
  head->ip = "1234";
  head->next = 0;

  int numClients = 1;

  addIfAbsent(head, "1111", &numClients);

  assert(numClients == 2);
  assert(strcmp(head->ip, "1234") == 0);
  assert(strcmp(head->next->ip, "1111") == 0);
  assert(head->next->next == 0);

  addIfAbsent(head, "1234", &numClients);

  assert(numClients == 2);
  assert(strcmp(head->ip, "1234") == 0);
  assert(strcmp(head->next->ip, "1111") == 0);
  assert(head->next->next == 0);
}