/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *    Copyright 2019 (c) Matthias Konnerth
 */

#include "backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dumpReference(void *context, TNode *node) {
  FILE *f = (FILE *)context;
  Reference *hierachicalRef = node->hierachicalRefs;
  while (hierachicalRef) {
    fprintf(f, "%d:%s,%d:%s,%d:%s\n", node->id.nsIdx, node->id.id,
            hierachicalRef->target.nsIdx, hierachicalRef->target.id,
            hierachicalRef->refType.nsIdx, hierachicalRef->refType.id);
    hierachicalRef = hierachicalRef->next;
  }

  Reference *nonHierRef = node->nonHierachicalRefs;
  while (nonHierRef) {
    fprintf(f, "%d:%s,%d:%s,%d:%s\n", node->id.nsIdx, node->id.id,
            nonHierRef->target.nsIdx, nonHierRef->target.id,
            nonHierRef->refType.nsIdx, nonHierRef->refType.id);
    nonHierRef = nonHierRef->next;
  }
}

static char *concat(const char *c1, const char *c2) {
  size_t len1 = strlen(c1);
  size_t len2 = strlen(c2);
  char *s = (char *)calloc(len1 + len2 + 1, sizeof(char));
  memcpy(s, c1, len1);
  memcpy(s + len1, c2, len2);
  return s;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("specify nodesetfiles as argument. E.g. exportToDsv text.xml\n");
    return 1;
  }
  char *outputDir = argv[1];
  

  
  FILE *refs = fopen(concat("./output/", "references"), "w");
  if (!refs) {
    printf("couldn't open refs - exit\n");
    return 1;
  }

  for (int i = 0; i < NODECLASS_COUNT; i++) {
    FILE *file = fopen(concat("./output/", NODECLASS_NAME[i]), "w");
    if (!file) {
      printf("could open %s file\n", NODECLASS_NAME[i]);
    }
    fprintf(file, "NodeId:ID,browseName\n");
    fclose(file);
  }

    struct Namespaces* uris = (struct Namespaces*)calloc(1, sizeof(struct Namespaces));
    uris->uris = (char**)calloc(1000, sizeof(char*));
    uris->uris[0] = "opcNamespace0";
    uris->size = 1;

  for (int cnt = 1; cnt < argc; cnt++) {
    NodesetLoader *loader = NodesetLoader_new(NULL, NULL);
    FileContext handler;
    handler.userContext = uris;
    handler.extensionHandling = NULL;
    handler.addNamespace = addNamespace;
    handler.file = argv[cnt];
    if (!NodesetLoader_importFile(loader, &handler)) {
      printf("nodeset could not be loaded, exit\n");
      return 1;
    }
    NodesetLoader_sort(loader);
    fprintf(refs, ":START_ID,:END_ID,:TYPE\n");

    for (int i = 0; i < NODECLASS_COUNT; i++) {
      TNode **nodes = NULL;
      FILE *file = fopen(concat("./output/", NODECLASS_NAME[i]), "a");
      NodesetLoader_forEachNode(loader, (TNodeClass)i, file, (NodesetLoader_forEachNode_Func)dumpNode);
      NodesetLoader_forEachNode(loader, (TNodeClass)i, refs,
                                (NodesetLoader_forEachNode_Func)dumpReference);
      fclose(file);
    }
    NodesetLoader_delete(loader);
  }
  fclose(refs);
  
  return 0;
}
