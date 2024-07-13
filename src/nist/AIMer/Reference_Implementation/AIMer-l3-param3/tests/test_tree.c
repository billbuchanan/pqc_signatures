// -----------------------------------------------------------------------------
// File Name   : test_tree.c
// Description : 
// SPDX-License-Identifier: MIT
// -----------------------------------------------------------------------------

#include "tree.h"
#include <stdio.h>

int main(void)
{
  uint8_t seed[16] =
            { 0,  1,  2,  3,  4,  5,  6,  7,
              8,  9, 10, 11, 12, 13, 14, 15};
  uint8_t salt[32] =
            {15, 14, 13, 12, 11, 10,  9,  8,
              7,  6,  5,  4,  3,  2,  1,  0,
              0,  0,  0,  0,  0,  0,  0,  0,
              0,  0,  0,  0,  0,  0,  0,  0};

  tree_t* tree = make_seed_tree(seed, 16, salt, 32, 64, 0);

  printf("[Tree is constructed.]\n");

  for (size_t idx = 0; idx < 64; idx++)
  {
    printf("Leaf #%02ld: ", idx);
    for (int j = 0; j < 16; j++)
    {
      printf("%02X ", get_leaf(tree, idx)[j]);
    }
    printf("\n");
  }

  reveal_list_t reveal_list = reveal_all_but(tree, 0);
  size_t data_node_count = ceil_log2(64);

  printf("\n[Reveal list is constructed.]\n");
  printf("Missing leaf index: %02ld\n", reveal_list.missing_leaf);
  printf("Reveal list size  : %02ld\n", data_node_count);

  for(size_t i = 0; i < data_node_count; i++)
  {
    printf("#%02ld: ", i);
    for(size_t j = 0; j < reveal_list.seed_size; j++)
    {
      printf("%02X ", reveal_list.data[i * reveal_list.seed_size + j]);
    }
    printf("\n");
  }

  tree_t* tree2 = reconstruct_seed_tree(&reveal_list, salt, 32, 64, 0);

  printf("\n[Reveal list can reconstruct tree.]\n");

  for (size_t idx = 0; idx < 64; idx++)
  {
    if(get_leaf(tree2, idx) == NULL)
    {
      printf("Leaf #%02ld: Empty!", idx);
    }
    else
    {
      printf("Leaf #%02ld: ", idx);
      int flag = 0;
      for (int j = 0; j < 16; j++)
      {
        uint8_t byte1 = get_leaf(tree, idx)[j];
        uint8_t byte2 = get_leaf(tree2, idx)[j];

        if(byte1 != byte2)
        {
          flag++;
        }

        printf("%02X ", byte2);
      }

      if(flag == 0)
      {
        printf("[OK!]");
      }
      else
      {
        printf("[Error!]");
      }
    }
    printf("\n");
  }
  
  free_tree(tree);
  free_tree(tree2);
  return 0;
}
