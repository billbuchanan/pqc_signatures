#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "params.h"
#include "coeff.h"
#include "poly.h"
#include "utils.h"

/*************************************************
* Name:        poly_transform_to_two_side
*
* Description: Transforms one sided shuffled typical set to two sided 
*              by flipping random entries
*
* Arguments:   - poly_n_2 *p: pointer to input/output polynomial
*              - uint16_t *current_pos: current pos in the polynomial
*              as one run might not be sufficient to iterate over the entire polynomial
*              - uint8_t buf[SHAKE256_RATE] buffer for shake output
*              to be used for polynomial sampling
**************************************************/
static void poly_transform_to_two_side(poly_n_2 *p, uint16_t *current_pos, uint8_t buf[SHAKE256_RATE])
{
  uint16_t i;
  coeff mask;
  for(i = 0; i < SHAKE256_RATE && (i*8 + *current_pos < N/2); i += 1)
  {
    for(int j = 0; j < 8; j++)
    {
      mask = -((buf[i] >> j) & 1);
      if(i*8 + *current_pos + j < N/2)
        p->coeffs[i*8 + *current_pos + j] = (mask & p->coeffs[i*8 + *current_pos + j]) | (~mask & coeff_red_modp((d_coeff)P - p->coeffs[i*8 + *current_pos + j]));
    }
  }
  *current_pos += i*8;
}

/*************************************************
* Name:        poly_sample_from_typical_set
*
* Description: Takes typical set for key, shuffles it with random bits from Keccak
*              and flips signs randomly to generate sk
*
* Arguments:   - poly_n_2 *p: pointer to input/output polynomial
*              - keccak_state *state: preinitialized keccak state with seed
*                to generate random bits
**************************************************/
void poly_sample_from_typical_set(poly_n_2 *p, keccak_state *state)
{
  //Declare necessary variables
  int i, j;
  uint8_t buf[SHAKE256_RATE];
  uint16_t range[N/2] = {0}; 

  //Define typical set based on param set
#if FULEECA_MODE == 1
  const coeff typical_set[N/2] = {65363, 65364, 65365, 65366, 65367, 65368, 65369, 65370, 65371, 65372, 65373, 65374, 65375, 65376, 65377, 65378, 65379, 65380, 65381, 65382, 65383, 65384, 65385, 65386, 65387, 65388, 65389, 65390, 65391, 65392, 65393, 65394, 65395, 65396, 65397, 65398, 65399, 65400, 65401, 65402, 65403, 65404, 65405, 65406, 65407, 65408, 65409, 65410, 65411, 65412, 65413, 65414, 65415, 65416, 65417, 65418, 65419, 65420, 65421, 65422, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 65439, 65440, 65441, 65442, 65443, 65444, 65445, 65446, 65447, 65448, 65449, 65450, 65451, 65452, 65453, 65454, 65455, 65456, 65457, 65458, 65459, 65460, 65461, 65462, 65463, 65463, 65464, 65464, 65465, 65465, 65466, 65466, 65467, 65467, 65468, 65468, 65469, 65469, 65470, 65470, 65471, 65471, 65472, 65472, 65473, 65473, 65474, 65474, 65475, 65475, 65476, 65476, 65477, 65477, 65478, 65478, 65479, 65479, 65480, 65480, 65481, 65481, 65482, 65482, 65482, 65483, 65483, 65483, 65484, 65484, 65484, 65485, 65485, 65485, 65486, 65486, 65486, 65487, 65487, 65487, 65488, 65488, 65488, 65489, 65489, 65489, 65490, 65490, 65490, 65491, 65491, 65491, 65492, 65492, 65492, 65493, 65493, 65493, 65494, 65494, 65494, 65495, 65495, 65495, 65495, 65496, 65496, 65496, 65496, 65497, 65497, 65497, 65497, 65498, 65498, 65498, 65498, 65499, 65499, 65499, 65499, 65500, 65500, 65500, 65500, 65501, 65501, 65501, 65501, 65502, 65502, 65502, 65502, 65503, 65503, 65503, 65503, 65504, 65504, 65504, 65504, 65505, 65505, 65505, 65505, 65506, 65506, 65506, 65506, 65506, 65507, 65507, 65507, 65507, 65507, 65508, 65508, 65508, 65508, 65508, 65509, 65509, 65509, 65509, 65509, 65510, 65510, 65510, 65510, 65510, 65511, 65511, 65511, 65511, 65511, 65512, 65512, 65512, 65512, 65512, 65513, 65513, 65513, 65513, 65513, 65514, 65514, 65514, 65514, 65514, 65514, 65515, 65515, 65515, 65515, 65515, 65515, 65516, 65516, 65516, 65516, 65516, 65516, 65517, 65517, 65517, 65517, 65517, 65517, 65518, 65518, 65518, 65518, 65518, 65518, 65519, 65519, 65519, 65519, 65519, 65519, 65520, 65520, 65520, 65520, 65520, 65520, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37, 37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#elif FULEECA_MODE == 3
  const coeff typical_set[N/2] = {65342, 65343, 65344, 65345, 65346, 65347, 65348, 65349, 65350, 65351, 65352, 65353, 65354, 65355, 65356, 65357, 65358, 65359, 65360, 65361, 65362, 65363, 65364, 65365, 65366, 65367, 65368, 65369, 65370, 65371, 65372, 65373, 65374, 65375, 65376, 65377, 65378, 65379, 65380, 65381, 65382, 65383, 65384, 65385, 65386, 65387, 65388, 65389, 65390, 65391, 65392, 65393, 65394, 65395, 65396, 65397, 65398, 65399, 65400, 65401, 65402, 65403, 65404, 65405, 65406, 65407, 65408, 65409, 65410, 65411, 65412, 65413, 65414, 65415, 65416, 65417, 65418, 65419, 65420, 65421, 65422, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 65430, 65431, 65432, 65433, 65434, 65435, 65436, 65437, 65438, 65439, 65440, 65441, 65442, 65443, 65443, 65444, 65444, 65445, 65445, 65446, 65446, 65447, 65447, 65448, 65448, 65449, 65449, 65450, 65450, 65451, 65451, 65452, 65452, 65453, 65453, 65454, 65454, 65455, 65455, 65456, 65456, 65457, 65457, 65458, 65458, 65459, 65459, 65460, 65460, 65461, 65461, 65462, 65462, 65462, 65463, 65463, 65463, 65464, 65464, 65464, 65465, 65465, 65465, 65466, 65466, 65466, 65467, 65467, 65467, 65468, 65468, 65468, 65469, 65469, 65469, 65470, 65470, 65470, 65471, 65471, 65471, 65472, 65472, 65472, 65473, 65473, 65473, 65474, 65474, 65474, 65475, 65475, 65475, 65476, 65476, 65476, 65476, 65477, 65477, 65477, 65477, 65478, 65478, 65478, 65478, 65479, 65479, 65479, 65479, 65480, 65480, 65480, 65480, 65481, 65481, 65481, 65481, 65482, 65482, 65482, 65482, 65483, 65483, 65483, 65483, 65484, 65484, 65484, 65484, 65485, 65485, 65485, 65485, 65486, 65486, 65486, 65486, 65486, 65487, 65487, 65487, 65487, 65487, 65488, 65488, 65488, 65488, 65488, 65489, 65489, 65489, 65489, 65489, 65490, 65490, 65490, 65490, 65490, 65491, 65491, 65491, 65491, 65491, 65492, 65492, 65492, 65492, 65492, 65493, 65493, 65493, 65493, 65493, 65494, 65494, 65494, 65494, 65494, 65495, 65495, 65495, 65495, 65495, 65495, 65496, 65496, 65496, 65496, 65496, 65496, 65497, 65497, 65497, 65497, 65497, 65497, 65498, 65498, 65498, 65498, 65498, 65498, 65499, 65499, 65499, 65499, 65499, 65499, 65500, 65500, 65500, 65500, 65500, 65500, 65501, 65501, 65501, 65501, 65501, 65501, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65503, 65503, 65503, 65503, 65503, 65503, 65503, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65505, 65505, 65505, 65505, 65505, 65505, 65505, 65506, 65506, 65506, 65506, 65506, 65506, 65506, 65507, 65507, 65507, 65507, 65507, 65507, 65507, 65508, 65508, 65508, 65508, 65508, 65508, 65508, 65509, 65509, 65509, 65509, 65509, 65509, 65509, 65509, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55, 56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#elif FULEECA_MODE == 5
  const coeff typical_set[N/2] = {65326, 65327, 65328, 65329, 65330, 65331, 65332, 65333, 65334, 65335, 65336, 65337, 65338, 65339, 65340, 65341, 65342, 65343, 65344, 65345, 65346, 65347, 65348, 65349, 65350, 65351, 65352, 65353, 65354, 65355, 65356, 65357, 65358, 65359, 65360, 65361, 65362, 65363, 65364, 65365, 65366, 65367, 65368, 65369, 65370, 65371, 65372, 65373, 65374, 65375, 65376, 65377, 65378, 65379, 65380, 65381, 65382, 65383, 65384, 65385, 65386, 65387, 65388, 65389, 65390, 65391, 65392, 65393, 65394, 65395, 65396, 65397, 65398, 65399, 65400, 65401, 65402, 65403, 65404, 65405, 65406, 65407, 65408, 65409, 65410, 65411, 65412, 65413, 65414, 65415, 65416, 65417, 65418, 65419, 65420, 65421, 65422, 65423, 65424, 65425, 65426, 65427, 65428, 65429, 65430, 65430, 65431, 65431, 65432, 65432, 65433, 65433, 65434, 65434, 65435, 65435, 65436, 65436, 65437, 65437, 65438, 65438, 65439, 65439, 65440, 65440, 65441, 65441, 65442, 65442, 65443, 65443, 65444, 65444, 65445, 65445, 65446, 65446, 65447, 65447, 65448, 65448, 65449, 65449, 65449, 65450, 65450, 65450, 65451, 65451, 65451, 65452, 65452, 65452, 65453, 65453, 65453, 65454, 65454, 65454, 65455, 65455, 65455, 65456, 65456, 65456, 65457, 65457, 65457, 65458, 65458, 65458, 65459, 65459, 65459, 65460, 65460, 65460, 65461, 65461, 65461, 65462, 65462, 65462, 65462, 65463, 65463, 65463, 65463, 65464, 65464, 65464, 65464, 65465, 65465, 65465, 65465, 65466, 65466, 65466, 65466, 65467, 65467, 65467, 65467, 65468, 65468, 65468, 65468, 65469, 65469, 65469, 65469, 65470, 65470, 65470, 65470, 65471, 65471, 65471, 65471, 65472, 65472, 65472, 65472, 65473, 65473, 65473, 65473, 65473, 65474, 65474, 65474, 65474, 65474, 65475, 65475, 65475, 65475, 65475, 65476, 65476, 65476, 65476, 65476, 65477, 65477, 65477, 65477, 65477, 65478, 65478, 65478, 65478, 65478, 65479, 65479, 65479, 65479, 65479, 65480, 65480, 65480, 65480, 65480, 65481, 65481, 65481, 65481, 65481, 65482, 65482, 65482, 65482, 65482, 65482, 65483, 65483, 65483, 65483, 65483, 65483, 65484, 65484, 65484, 65484, 65484, 65484, 65485, 65485, 65485, 65485, 65485, 65485, 65486, 65486, 65486, 65486, 65486, 65486, 65487, 65487, 65487, 65487, 65487, 65487, 65488, 65488, 65488, 65488, 65488, 65488, 65489, 65489, 65489, 65489, 65489, 65489, 65489, 65490, 65490, 65490, 65490, 65490, 65490, 65490, 65491, 65491, 65491, 65491, 65491, 65491, 65491, 65492, 65492, 65492, 65492, 65492, 65492, 65492, 65493, 65493, 65493, 65493, 65493, 65493, 65493, 65494, 65494, 65494, 65494, 65494, 65494, 65494, 65495, 65495, 65495, 65495, 65495, 65495, 65495, 65495, 65496, 65496, 65496, 65496, 65496, 65496, 65496, 65496, 65497, 65497, 65497, 65497, 65497, 65497, 65497, 65497, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65498, 65499, 65499, 65499, 65499, 65499, 65499, 65499, 65499, 65500, 65500, 65500, 65500, 65500, 65500, 65500, 65500, 65501, 65501, 65501, 65501, 65501, 65501, 65501, 65501, 65501, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65502, 65503, 65503, 65503, 65503, 65503, 65503, 65503, 65503, 65503, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65504, 65505, 65505, 65505, 65505, 65505, 65505, 65505, 65505, 65505, 65506, 65506, 65506, 65506, 65506, 65506, 65506, 65506, 65506, 65506, 65507, 65507, 65507, 65507, 65507, 65507, 65507, 65507, 65507, 65507, 65508, 65508, 65508, 65508, 65508, 65508, 65508, 65508, 65508, 65508, 65509, 65509, 65509, 65509, 65509, 65509, 65509, 65509, 65509, 65509, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65510, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65511, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65512, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65513, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65514, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65515, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65516, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65517, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65518, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65519, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 65520, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 46, 46, 46, 46, 46, 47, 47, 47, 47, 47, 48, 48, 48, 48, 48, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55, 55, 56, 56, 56, 56, 57, 57, 57, 57, 58, 58, 58, 58, 59, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 65, 65, 65, 66, 66, 66, 67, 67, 67, 68, 68, 68, 69, 69, 69, 70, 70, 70, 71, 71, 71, 72, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80, 81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 91, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

  //Init and shuffle range array
  shuffle_array_indices(state, range);

  // Still need to assign indices to array
  for(i = 0; i < N/2; i++)
  {
    for(j = 0; j < N/2; j++)
    {
      p->coeffs[i] ^= (typical_set[j] & -(range[i] == j));
    }
  }

  //Use random bits to flip signs of coeffs. in key
  uint16_t current_pos = 0;
  while(current_pos < N/2)
  {
    shake256_squeezeblocks(buf, 1, state);
    poly_transform_to_two_side(p, &current_pos, buf);
  }
}

/*************************************************
* Name:        extended_euclidean_algorithm
*
* Description: Takes a polynomial of lenght N/2 and computes the multiplicative inverse with respect to the polynomial (-1)^N +1
*              This function does not check whether the returned polynomial really is an inverse
*
* Arguments:   - poly_n_2 *a_inv: pointer to the inverted polynomial 
*              - poly_n_2 *a: pointer to the polynomial to be inverted
**************************************************/
//According to ia.cr/2019/266
void extended_euclidean_algorithm(poly_n_2 *a_inv, poly_n_2 *a)
{
  //Declare variables
  poly_inv a_reverse = { 0 };
  poly_inv poly_b = { 0 };
  poly_inv *a_rev = &a_reverse;
  poly_inv *b = &poly_b;
  poly_inv poly_v = {0}, poly_r = {0}, poly_r_tmp = { 0 };
  poly_inv *v = &poly_v;
  poly_inv *r = &poly_r;
  poly_inv *r_tmp = &poly_r_tmp;
  uint16_t div_v = 0, div_r = 0;
  poly_inv *tmp;
  uint16_t div_tmp;
  coeff a_rev_0, b_0;
  poly_n v_w_zero = {0};

  //Reverse the order of coefficients in b; b_reverse = x^d*b(1/x) and a; a=reverse = x^(d-1)*a(1/x),
  b->coeffs[0] = 1;
  b->coeffs[N/2] = P - 1;
  for (int i = 0; i < N/2; i++) {
    a_rev->coeffs[N/2 - 1 - i] = a->coeffs[i];
  }
  //Set starting value for EEA
  r->coeffs[0] = 1;
  int16_t delta = 1;
  
  for(int i = N - 1; i > 0; i--)
  {
    //The following block implmements this swapping operation in constant time
    /*if(delta > 0 && a_rev->coeffs[0] != 0)
    {
      delta = -delta;
      tmp = a_rev;
      a_rev = b;
      b = tmp;
      tmp = q;
      div_tmp = div_q;
      q = u;
      div_q = div_u;
      u = tmp;
      div_u = div_tmp;
      tmp = v;
      div_tmp = div_v;
      v = r;
      div_v = div_r;
      r = tmp;
      div_r = div_tmp;
    }*/
    uint8_t g_bit = 0;
    for(long unsigned int j = 0; j < sizeof(coeff)*8; j++)
    {
      g_bit |= a_rev->coeffs[0] >> j;
    }
    bool sign = delta > 0;
    bool switch_bit = sign & (g_bit & 1);
    delta = (delta ^ -switch_bit) + switch_bit;
    tmp = a_rev;
    a_rev = (poly_inv *) (((uintptr_t) a_rev & ~(-switch_bit)) | ((uintptr_t) b & -switch_bit));
    b = (poly_inv *) (((uintptr_t) b & ~(-switch_bit)) | ((uintptr_t) tmp & -switch_bit));
    tmp = v;
    v = (poly_inv *) (((uintptr_t) v & ~(-switch_bit)) | ((uintptr_t) r & -switch_bit));
    r = (poly_inv *) (((uintptr_t) r & ~(-switch_bit)) | ((uintptr_t) tmp & -switch_bit));
    div_tmp = div_v;
    div_v = (div_v & ~(-switch_bit)) | (div_r & -switch_bit);
    div_r = (div_r & ~(-switch_bit)) | (div_tmp & -switch_bit);
    //Increment delta
    delta++;
    //Store the lsb coefficients of a and b
    a_rev_0 = a_rev->coeffs[0];
    b_0 = b->coeffs[0];
    //Calculate new a
    for(int j = 1; j < N; j++) {
      a_rev->coeffs[j-1] = coeff_sub_modp(coeff_mul_modp(b_0, a_rev->coeffs[j]), coeff_mul_modp(a_rev_0, b->coeffs[j]));
    }
    //Set r_tmp to zero
    memset(r_tmp->coeffs, 0, sizeof(r_tmp->coeffs));

    //Store a temporary round counter
    int tmp_runs = N - i - 1;
    if(i >= N - 1){
      tmp_runs = 1;
    }
    //Calculate the new r_tmp
    for(int j = 0; j < tmp_runs; j++)
    {
      r_tmp->coeffs[j + (N-i) - 1 - div_r] = coeff_add_modp(r_tmp->coeffs[j + (N-i) - 1 - div_r], coeff_mul_modp(b_0, r->coeffs[j]));
      r_tmp->coeffs[j + (N-i) - 1 - div_v] = coeff_sub_modp(r_tmp->coeffs[j + (N-i) - 1 - div_v], coeff_mul_modp(a_rev_0, v->coeffs[j]));
    }
    //Replace the old r with the new r_tmp (pointer swap)
    tmp = r;
    r = r_tmp;
    r_tmp = tmp;
    div_r = N - i;
  }
  //Calculate the inverse of the last coefficient of b with fermats little theorem
  coeff inverse = b->coeffs[0];
  for(int i = 0; i < P - 3; i++)
  {
    inverse = coeff_mul_modp(inverse, b->coeffs[0]);
  }

  //The following two loops extract the inverse from v_w_zero with a fixed number of instructions
  for(int i = 0; i < N/2; i++)
  {
    v_w_zero.coeffs[N/2 + i] = v->coeffs[i];
  }
  for(int i = 0; i < N/2; i++)
  {
    a_inv->coeffs[i] = coeff_mul_modp(inverse, v_w_zero.coeffs[(((N) - (N - 2 - div_v)) - (i + 1))]);
  }
}

/*************************************************
* Name:        rel_key_address
*
* Description: Calculate the relative address of an element for a line of Gsec, i.e.
*              this function implements a cyclic shift by translating the row and column index in the matrix
*              to the corresponding index in the key polynomial of length N/2, i.e. either a or b.
*              Curr. key index = j - i; if j - i < 0: N/2 - i + j
*
* Arguments:   - size_t key_line: Current row/line in Gsec
*              - size_t rel_position: Current column in Gsec
*              - return: Returns the corresponding polynomial index
**************************************************/
size_t rel_key_address(size_t key_line, size_t rel_position) {
  return ((rel_position - key_line) & ~(-(rel_position < key_line))) | ((N/2 - key_line + rel_position) & -(rel_position < key_line));
}

/*************************************************
* Name:        calc_key_support
*
* Description: Calculate the support of the key polynomials a and b, i.e. their number of non-zero elements
*
* Arguments:   - const poly_n_2 *a: sk polynomial a
*              - const poly_n_2 *b: sk polynomial b
*              - return: Returns the support of the key
**************************************************/
size_t calc_key_support(const poly_n_2 *a, const poly_n_2 *b)
{
  size_t key_support = 0, i;
  for(i = 0; i < N/2; i++)
  {
    key_support += a->coeffs[i] != 0;
    key_support += b->coeffs[i] != 0;
  }
  return key_support;
}

/*************************************************
* Name:        calc_sig_support_poly_n
*
* Description: Calculate the support of a signature v, i.e. its number of non-zero elements
*
* Arguments:   - const poly_n *v: signature candidate v
*              - return: Returns the support of the signature candidate
**************************************************/
size_t calc_sig_support_poly_n(const poly_n *v)
{
        size_t sig_support = 0, i;
        for(i = 0; i < N; i++)
        {
          sig_support += v->coeffs[i] != 0;
        }
        return sig_support;
}

/*************************************************
* Name:        calc_sig_support_poly_s_d_n
*
* Description: Calculate the support of a signature v, i.e. its number of non-zero elements
*
* Arguments:   - const poly_s_d_n *v: signature candidate v
*              - return: Returns the support of the signature candidate
**************************************************/
size_t calc_sig_support_poly_s_d_n(const poly_s_d_n *v)
{
        size_t sig_support = 0, i;
        for(i = 0; i < N; i++)
        {
    sig_support += v->coeffs[i] != 0;
        }
        return sig_support;
}

/*************************************************
* Name:        simple_sign_score
*
* Description: Calculate how often which line of the key is to be added or subtracted to generate
*              the initial signature candidate
*
* Arguments:   - int *score_per_row: Factor by which this row is to be multiplied before being added
*              - const poly_n_2 *a: sk polynomial a
*              - const poly_n_2 *b: sk polynomial b
*              - const uint8_t *mhash: The challenge to match against
**************************************************/
void simple_sign_score(int *score_per_row, const poly_n_2 *a, const poly_n_2 *b, const uint8_t *mhash)
{
  //Calculate the support of the key vector
  int key_support = calc_key_support(a, b);

  //Declare variables
  uint16_t sign_matches[N/2] = {0};
  uint8_t sign = 0;
  uint16_t abs_value = 0;
  int16_t orig_value = 0;
  uint16_t res_value = 0;
  size_t curr_chall_bit, curr_chall_bit_offset, curr_chall_index, curr_chall_index_offset, curr_key_index;
  size_t i, j;

  //Iterate over Gsec
  //Outer iterator over all rows of Gsec
  for(i = 0; i < N/2; i++)
  {
    //Inner iterator over all coeffs in one keyline; polynomial a and b as well add and subtract simultaneously
    for(j = 0; j < N/2; j++)
    {
      //Implement cyclic shift by altered index
      curr_key_index = rel_key_address(i, j);
      //Index the challenge bitwise
      curr_chall_index = j >> 3;
      curr_chall_bit = j & 7;
      curr_chall_index_offset = (N/2 + j) >> 3;
      curr_chall_bit_offset = (N/2 + j) & 7;
      //Check whether a one in the challenge matches a negative (i.e. larger than (P-1)/2) coeff in the key or a zero in the challenge matches a non-zero coeff smaller or egual to (P-1)/2) for a and b
      sign_matches[i] += (((a->coeffs[curr_key_index] > (P-1)/2) & (mhash[curr_chall_index] >> curr_chall_bit)) & 1);
      sign_matches[i] += (((b->coeffs[curr_key_index] > (P-1)/2) & (mhash[curr_chall_index_offset] >> curr_chall_bit_offset)) & 1);
      sign_matches[i] += (((a->coeffs[curr_key_index] <= (P-1)/2) & (a->coeffs[curr_key_index] > 0) & (((mhash[curr_chall_index] >> curr_chall_bit) & 1) == 0)) & 1);
      sign_matches[i] += (((b->coeffs[curr_key_index] <= (P-1)/2) & (b->coeffs[curr_key_index] > 0) & (((mhash[curr_chall_index_offset] >> curr_chall_bit_offset) & 1) == 0)) & 1);
    }
  }
  //Divide the key support by two
  key_support = key_support >> 1;
  //iterate over all found match counts from above and scale as defined by MULT_X and DIV_V
  for(i = 0; i < N/2; i++)
  {
    orig_value = (int)(sign_matches[i] - key_support);
    sign = (orig_value < 0);
    abs_value = ((uint16_t)orig_value & (~(-sign))) | ((uint16_t)(orig_value * -1) & (-sign));
    res_value = (abs_value * MULT_X) >> DIV_X;
    score_per_row[i] = ((int)res_value & (~(-sign))) | (((int)res_value * -1) & (-sign));
  }
}

/*************************************************
* Name:        calc_matches_and_rel_weight_verify
*
* Description: Calculate lmp and lee weight of a to be published or to be verified signature
*
* Arguments:   - int32_t *lmp: Pointer to the calculated lmp
*              - size_t *lee_weight: Pointer to the calculated lee weight
*              - const poly_n *sig: Signature candidate to be checked
*              - const uint8_t *mhash: Challenge the signature is to be checked against
**************************************************/
void calc_matches_and_rel_weight_verify(int32_t *lmp, size_t *lee_weight, const poly_n *sig, const uint8_t *mhash) {
  //Declare variables
  size_t i, curr_chall_index, curr_chall_bit, matches, sig_support;
  
  //Set lee weight to zero
  *lee_weight = 0;

  //Set matches to zero
  matches = 0;
  
  //Iterate over all coeffs. in the signature
  for(i = 0; i < N; i++)
  {
    //Determine the current challenge bit index
    curr_chall_index = i >> 3;
    curr_chall_bit = i & 7;
    //Calculate the total lee weight by summing up the lee weight of all coeffs
    *lee_weight += coeff_lee_weight(sig->coeffs[i]);

    //Calculate the number of matches by comparing sign of coeffs with bit of challenge
    matches += (sig->coeffs[i] > P/2 && ((mhash[curr_chall_index] >> curr_chall_bit) & 1));
    matches += (sig->coeffs[i] <= P/2 && sig->coeffs[i] > 0 && (((mhash[curr_chall_index] >> curr_chall_bit) & 1) == 0));
  }
  //Calculate the support of the signature
  sig_support = calc_sig_support_poly_n(sig);

  //Calculate the resulting lmp
  *lmp = -lmp_score(matches, sig_support);
}

/*************************************************
* Name:        gen_signature_candidate
*
* Description: Generate an initial signature candidate based on score_per_row
*
* Arguments:   - poly_n *v: Pointer to the to be calculated candidate
*              - const poly_n_2 *a: pointer to sk polynomial a
*              - const poly_n_2 *b: pointer to sk polynomial b
*              - const int *score_per_row: Scaling factor to multiply the row with before adding
**************************************************/
void gen_signature_candidate(poly_n *v, const poly_n_2 *a, const poly_n_2 *b, const int *score_per_row) {
  //Declare variables
  size_t i, j, curr_key_index;
  coeff factor;
  
  //Iterate over all rows of Gsec, i.e. all cyclic shifts of the key
  for(i = 0; i < N/2; i++)
  {
    //Calculate the unsigned scaling factor for the current row
    factor = (coeff_red_modp((d_coeff)(P + score_per_row[i])) & (-(score_per_row[i] < 0))) | (score_per_row[i] & ~(-(score_per_row[i] < 0)));
    
    //Iterate over all coeffs in the current row
    for(j = 0; j < N/2; j++)
    {
      //Calculate the key index corresponding to a cyclic shift
      curr_key_index = rel_key_address(i, j);

      //Calculate the signature candidate by adding the new scaled row of the key
      v->coeffs[j] = coeff_add_modp(v->coeffs[j], coeff_mul_modp(a->coeffs[curr_key_index], factor));
      v->coeffs[N/2 + j] = coeff_add_modp(v->coeffs[N/2 + j], coeff_mul_modp(b->coeffs[curr_key_index], factor));
    }
  }
}

/*************************************************
* Name:        find_improvement
*
* Description: Try adding subtracting all rows of Gsec to improve the current sig. candidate
*
* Arguments:   - poly_s_d_n *v: Pointer to the signed representation of the current sig. candidate
*              - uint8_t *allowed_indexes: Array which defines whether it is allowed to add/subtract a row based on the previous adds/subs
*              - const poly_s_n gsec[N/2]: Expanded sk as Gsec, i.e. all cyclic shifts of a and b
*              - const uint8_t *exp_chall: Challenge expanded with one byte per bit of the challenge
*              - int32_t *return_lmp: Resulting LMP for the next iteration
*              - const uint8_t loopfree: Define whether the value in allowed_indexes is ignored
**************************************************/
void find_improvement(poly_s_d_n *v, uint8_t *allowed_indexes, const poly_s_n gsec[N/2], const uint8_t *exp_chall, int32_t *return_lmp, const uint8_t loopfree) {
  //Declare variables
  size_t i, j;
  poly_s_d_n poly_v_bar, poly_v_bar_pos, poly_v_bar_neg;
  poly_s_d_n *v_bar = &poly_v_bar, *v_bar_pos = &poly_v_bar_pos, *v_bar_neg = &poly_v_bar_neg;
  int32_t cand_lmp, ret_lmp;
  size_t mcnt_pos, mcnt_neg, blocked_index;
  //NOTE: Set to value larger than largest value in LMP LUT
  cand_lmp = 20000000;
  ret_lmp = 0;
  blocked_index = 0;

  //Iterate over all possible key rows
  for(i = 0; i < N/2; i++)
  {
    //Calculate two new candidates (add/sub row)
    for (j = 0; j < N; j++)
    {
      v_bar_pos->coeffs[j] = v->coeffs[j] + gsec[i].coeffs[j];
      v_bar_neg->coeffs[j] = v->coeffs[j] - gsec[i].coeffs[j];
    }
    //Calculate match count of the two new candidates
    mcnt_pos = 0;
    mcnt_neg = 0;
    for (j = 0; j < N; j++)
    {
      mcnt_pos += ((v_bar_pos->coeffs[j] != 0) && !((v_bar_pos->coeffs[j] < 0) ^ exp_chall[j]));
      mcnt_neg += ((v_bar_neg->coeffs[j] != 0) && !((v_bar_neg->coeffs[j] < 0) ^ exp_chall[j]));
    }
    //Conditional update of the candidate with the newly generated by adding the current key row 
    update_cand(&v_bar, &v_bar_pos, &ret_lmp, &cand_lmp, allowed_indexes, loopfree, &blocked_index, mcnt_pos, i, i + N/2);
    
    //Conditional update of the candidate with the newly generated by subtracting the current key row 
    update_cand(&v_bar, &v_bar_neg, &ret_lmp, &cand_lmp, allowed_indexes, loopfree, &blocked_index, mcnt_neg, i + N/2, i);
  }
  //Change the opposite (add for subtract and vice versa) allowance bit in allowed indexes to prevent undoing of the just done operation
  allowed_indexes[blocked_index] = 0;

  //Copy the new candidate to the output buffer
  memcpy(v->coeffs, v_bar->coeffs, sizeof(v->coeffs));

  //Set the LMP for the next iteration
  *return_lmp = ret_lmp;
}


/*************************************************
* Name:        update_cand
*
* Description: Conditional update of the current signature candidate
*
* Arguments:   - poly_s_d_n **v_bar: Pointer to the pointer to v_bar to allow swapping 
*              - poly_s_d_n **v_bar_new: Pointer to the pointer to v_bar_new to allow swapping
*              - int32_t *ret_lmp: LMP to be returned
*              - int32_t *cand_lmp: LMP of the candidate for future comp.
*              - const uint8_t *allowed_indexes: Array of allowed abb/sub operations
*              - const uint8_t loopfree: Determine whether add/subs can be undone
*              - size_t *blocked_index: Return value of the new to blocked op. for allowed_indexes
*              - size_t mcnt: Match count of this candidate
*              - size_t curr_i: Current row of Gsec
*              - size_t target_i: Row of Gsec with opposite sign
*              - const poly_n_2 *b: pointer to sk polynomial b
*              - const int *score_per_row: Scaling factor to multiply the row with before adding
**************************************************/
void update_cand(poly_s_d_n **v_bar, poly_s_d_n **v_bar_new, int32_t *ret_lmp, int32_t *cand_lmp, const uint8_t *allowed_indexes, const uint8_t loopfree, size_t *blocked_index, size_t mcnt, size_t curr_i, size_t target_i)
{
  //Declare variables
  size_t sig_support;
  int32_t curr_lmp, check_lmp;
  int switch_bit, not_switch_bit;
  poly_s_d_n *v_tmp;

  //Calculate support of candidate and corresponding LMPs
  sig_support = calc_sig_support_poly_s_d_n(*v_bar_new);
  check_lmp = -lmp_score(mcnt, sig_support);
  curr_lmp = abs(check_lmp - LAMBDA - 65);

  // The following cond. swap is implemented in constant time below and left as if for readability
  /*if(curr_lmp <= cand_lmp && allowed_indexes[i] == 1 || loopfree)
  {
    v_tmp         = v_bar;
    v_bar         = v_bar_pos;
    v_bar_pos     = v_tmp;
    blocked_index = N/2 + i;
    cand_lmp      = curr_lmp;
    ret_lmp       = check_lmp;
  }*/
  switch_bit     = -(curr_lmp <= (*cand_lmp) && ((allowed_indexes[curr_i] == 1) || loopfree));
  not_switch_bit = ~switch_bit;
  v_tmp          = *v_bar;
  *v_bar         = (poly_s_d_n *) (((uintptr_t) *v_bar & not_switch_bit) | ((uintptr_t) *v_bar_new & switch_bit));
  *v_bar_new     = (poly_s_d_n *) (((uintptr_t) *v_bar_new & not_switch_bit) | ((uintptr_t) v_tmp & switch_bit));
  *blocked_index = (*blocked_index & not_switch_bit) | (target_i & switch_bit);
  *cand_lmp      = (*cand_lmp & not_switch_bit) | (curr_lmp & switch_bit);
  *ret_lmp       = (*ret_lmp & not_switch_bit) | (check_lmp & switch_bit);
}

/*************************************************
* Name:        lmp_score
*
* Description: Find the LMP in a look up table based on the support size of a candidate and its match count with the challenge
*
* Arguments:   - size_t mcnt: Match count of candidate and challenge
*              - size_t support_size: Support Size of the Candidate
**************************************************/
int32_t lmp_score(size_t mcnt, size_t support_size)
{
  //Declare variables
  int32_t lmp;

  //Define LUT for log_prob
  uint16_t logfact[4096] = {0, 0, 1, 3, 5, 7, 9, 12, 15, 18, 22, 25, 29, 33, 36, 40, 44, 48, 53, 57, 61, 65, 70, 74, 79, 84, 88, 93, 98, 103, 108, 113, 118, 123, 128, 133, 138, 143, 149, 154, 159, 165, 170, 175, 181, 186, 192, 197, 203, 209, 214, 220, 226, 231, 237, 243, 249, 254, 260, 266, 272, 278, 284, 290, 296, 302, 308, 314, 320, 326, 332, 339, 345, 351, 357, 363, 370, 376, 382, 389, 395, 401, 408, 414, 420, 427, 433, 440, 446, 453, 459, 466, 472, 479, 485, 492, 498, 505, 511, 518, 525, 531, 538, 545, 551, 558, 565, 572, 578, 585, 592, 599, 606, 612, 619, 626, 633, 640, 647, 654, 660, 667, 674, 681, 688, 695, 702, 709, 716, 723, 730, 737, 744, 751, 758, 765, 773, 780, 787, 794, 801, 808, 815, 822, 830, 837, 844, 851, 858, 866, 873, 880, 887, 895, 902, 909, 916, 924, 931, 938, 946, 953, 960, 968, 975, 982, 990, 997, 1005, 1012, 1019, 1027, 1034, 1042, 1049, 1057, 1064, 1071, 1079, 1086, 1094, 1101, 1109, 1116, 1124, 1131, 1139, 1147, 1154, 1162, 1169, 1177, 1184, 1192, 1200, 1207, 1215, 1222, 1230, 1238, 1245, 1253, 1261, 1268, 1276, 1284, 1291, 1299, 1307, 1314, 1322, 1330, 1338, 1345, 1353, 1361, 1369, 1376, 1384, 1392, 1400, 1408, 1415, 1423, 1431, 1439, 1447, 1454, 1462, 1470, 1478, 1486, 1494, 1501, 1509, 1517, 1525, 1533, 1541, 1549, 1557, 1565, 1573, 1580, 1588, 1596, 1604, 1612, 1620, 1628, 1636, 1644, 1652, 1660, 1668, 1676, 1684, 1692, 1700, 1708, 1716, 1724, 1732, 1740, 1748, 1756, 1764, 1772, 1780, 1789, 1797, 1805, 1813, 1821, 1829, 1837, 1845, 1853, 1861, 1870, 1878, 1886, 1894, 1902, 1910, 1918, 1927, 1935, 1943, 1951, 1959, 1967, 1976, 1984, 1992, 2000, 2008, 2017, 2025, 2033, 2041, 2050, 2058, 2066, 2074, 2082, 2091, 2099, 2107, 2116, 2124, 2132, 2140, 2149, 2157, 2165, 2174, 2182, 2190, 2199, 2207, 2215, 2223, 2232, 2240, 2249, 2257, 2265, 2274, 2282, 2290, 2299, 2307, 2315, 2324, 2332, 2341, 2349, 2357, 2366, 2374, 2383, 2391, 2399, 2408, 2416, 2425, 2433, 2442, 2450, 2459, 2467, 2475, 2484, 2492, 2501, 2509, 2518, 2526, 2535, 2543, 2552, 2560, 2569, 2577, 2586, 2594, 2603, 2611, 2620, 2628, 2637, 2645, 2654, 2663, 2671, 2680, 2688, 2697, 2705, 2714, 2723, 2731, 2740, 2748, 2757, 2765, 2774, 2783, 2791, 2800, 2808, 2817, 2826, 2834, 2843, 2852, 2860, 2869, 2877, 2886, 2895, 2903, 2912, 2921, 2929, 2938, 2947, 2955, 2964, 2973, 2981, 2990, 2999, 3008, 3016, 3025, 3034, 3042, 3051, 3060, 3068, 3077, 3086, 3095, 3103, 3112, 3121, 3130, 3138, 3147, 3156, 3165, 3173, 3182, 3191, 3200, 3208, 3217, 3226, 3235, 3244, 3252, 3261, 3270, 3279, 3287, 3296, 3305, 3314, 3323, 3332, 3340, 3349, 3358, 3367, 3376, 3385, 3393, 3402, 3411, 3420, 3429, 3438, 3446, 3455, 3464, 3473, 3482, 3491, 3500, 3509, 3517, 3526, 3535, 3544, 3553, 3562, 3571, 3580, 3589, 3598, 3606, 3615, 3624, 3633, 3642, 3651, 3660, 3669, 3678, 3687, 3696, 3705, 3714, 3723, 3732, 3740, 3749, 3758, 3767, 3776, 3785, 3794, 3803, 3812, 3821, 3830, 3839, 3848, 3857, 3866, 3875, 3884, 3893, 3902, 3911, 3920, 3929, 3938, 3947, 3956, 3965, 3974, 3983, 3992, 4001, 4011, 4020, 4029, 4038, 4047, 4056, 4065, 4074, 4083, 4092, 4101, 4110, 4119, 4128, 4137, 4146, 4156, 4165, 4174, 4183, 4192, 4201, 4210, 4219, 4228, 4237, 4247, 4256, 4265, 4274, 4283, 4292, 4301, 4310, 4320, 4329, 4338, 4347, 4356, 4365, 4374, 4384, 4393, 4402, 4411, 4420, 4429, 4438, 4448, 4457, 4466, 4475, 4484, 4494, 4503, 4512, 4521, 4530, 4539, 4549, 4558, 4567, 4576, 4585, 4595, 4604, 4613, 4622, 4631, 4641, 4650, 4659, 4668, 4678, 4687, 4696, 4705, 4715, 4724, 4733, 4742, 4752, 4761, 4770, 4779, 4789, 4798, 4807, 4816, 4826, 4835, 4844, 4853, 4863, 4872, 4881, 4891, 4900, 4909, 4918, 4928, 4937, 4946, 4956, 4965, 4974, 4983, 4993, 5002, 5011, 5021, 5030, 5039, 5049, 5058, 5067, 5077, 5086, 5095, 5105, 5114, 5123, 5133, 5142, 5151, 5161, 5170, 5179, 5189, 5198, 5208, 5217, 5226, 5236, 5245, 5254, 5264, 5273, 5282, 5292, 5301, 5311, 5320, 5329, 5339, 5348, 5358, 5367, 5376, 5386, 5395, 5405, 5414, 5423, 5433, 5442, 5452, 5461, 5470, 5480, 5489, 5499, 5508, 5518, 5527, 5536, 5546, 5555, 5565, 5574, 5584, 5593, 5603, 5612, 5621, 5631, 5640, 5650, 5659, 5669, 5678, 5688, 5697, 5707, 5716, 5726, 5735, 5745, 5754, 5764, 5773, 5782, 5792, 5801, 5811, 5820, 5830, 5839, 5849, 5858, 5868, 5877, 5887, 5896, 5906, 5916, 5925, 5935, 5944, 5954, 5963, 5973, 5982, 5992, 6001, 6011, 6020, 6030, 6039, 6049, 6058, 6068, 6078, 6087, 6097, 6106, 6116, 6125, 6135, 6144, 6154, 6164, 6173, 6183, 6192, 6202, 6211, 6221, 6231, 6240, 6250, 6259, 6269, 6279, 6288, 6298, 6307, 6317, 6327, 6336, 6346, 6355, 6365, 6375, 6384, 6394, 6403, 6413, 6423, 6432, 6442, 6451, 6461, 6471, 6480, 6490, 6500, 6509, 6519, 6529, 6538, 6548, 6557, 6567, 6577, 6586, 6596, 6606, 6615, 6625, 6635, 6644, 6654, 6664, 6673, 6683, 6693, 6702, 6712, 6722, 6731, 6741, 6751, 6760, 6770, 6780, 6789, 6799, 6809, 6818, 6828, 6838, 6848, 6857, 6867, 6877, 6886, 6896, 6906, 6915, 6925, 6935, 6945, 6954, 6964, 6974, 6983, 6993, 7003, 7013, 7022, 7032, 7042, 7052, 7061, 7071, 7081, 7090, 7100, 7110, 7120, 7129, 7139, 7149, 7159, 7168, 7178, 7188, 7198, 7207, 7217, 7227, 7237, 7247, 7256, 7266, 7276, 7286, 7295, 7305, 7315, 7325, 7334, 7344, 7354, 7364, 7374, 7383, 7393, 7403, 7413, 7423, 7432, 7442, 7452, 7462, 7472, 7481, 7491, 7501, 7511, 7521, 7530, 7540, 7550, 7560, 7570, 7579, 7589, 7599, 7609, 7619, 7629, 7638, 7648, 7658, 7668, 7678, 7688, 7697, 7707, 7717, 7727, 7737, 7747, 7757, 7766, 7776, 7786, 7796, 7806, 7816, 7825, 7835, 7845, 7855, 7865, 7875, 7885, 7895, 7904, 7914, 7924, 7934, 7944, 7954, 7964, 7974, 7983, 7993, 8003, 8013, 8023, 8033, 8043, 8053, 8063, 8072, 8082, 8092, 8102, 8112, 8122, 8132, 8142, 8152, 8162, 8172, 8181, 8191, 8201, 8211, 8221, 8231, 8241, 8251, 8261, 8271, 8281, 8291, 8301, 8310, 8320, 8330, 8340, 8350, 8360, 8370, 8380, 8390, 8400, 8410, 8420, 8430, 8440, 8450, 8460, 8470, 8480, 8490, 8500, 8509, 8519, 8529, 8539, 8549, 8559, 8569, 8579, 8589, 8599, 8609, 8619, 8629, 8639, 8649, 8659, 8669, 8679, 8689, 8699, 8709, 8719, 8729, 8739, 8749, 8759, 8769, 8779, 8789, 8799, 8809, 8819, 8829, 8839, 8849, 8859, 8869, 8879, 8889, 8899, 8909, 8919, 8929, 8939, 8949, 8959, 8969, 8979, 8989, 8999, 9009, 9019, 9029, 9040, 9050, 9060, 9070, 9080, 9090, 9100, 9110, 9120, 9130, 9140, 9150, 9160, 9170, 9180, 9190, 9200, 9210, 9220, 9231, 9241, 9251, 9261, 9271, 9281, 9291, 9301, 9311, 9321, 9331, 9341, 9351, 9361, 9372, 9382, 9392, 9402, 9412, 9422, 9432, 9442, 9452, 9462, 9472, 9483, 9493, 9503, 9513, 9523, 9533, 9543, 9553, 9563, 9573, 9584, 9594, 9604, 9614, 9624, 9634, 9644, 9654, 9664, 9675, 9685, 9695, 9705, 9715, 9725, 9735, 9745, 9756, 9766, 9776, 9786, 9796, 9806, 9816, 9827, 9837, 9847, 9857, 9867, 9877, 9887, 9898, 9908, 9918, 9928, 9938, 9948, 9959, 9969, 9979, 9989, 9999, 10009, 10020, 10030, 10040, 10050, 10060, 10070, 10081, 10091, 10101, 10111, 10121, 10131, 10142, 10152, 10162, 10172, 10182, 10193, 10203, 10213, 10223, 10233, 10243, 10254, 10264, 10274, 10284, 10294, 10305, 10315, 10325, 10335, 10345, 10356, 10366, 10376, 10386, 10396, 10407, 10417, 10427, 10437, 10448, 10458, 10468, 10478, 10488, 10499, 10509, 10519, 10529, 10540, 10550, 10560, 10570, 10580, 10591, 10601, 10611, 10621, 10632, 10642, 10652, 10662, 10673, 10683, 10693, 10703, 10714, 10724, 10734, 10744, 10755, 10765, 10775, 10785, 10796, 10806, 10816, 10826, 10837, 10847, 10857, 10867, 10878, 10888, 10898, 10909, 10919, 10929, 10939, 10950, 10960, 10970, 10980, 10991, 11001, 11011, 11022, 11032, 11042, 11052, 11063, 11073, 11083, 11094, 11104, 11114, 11124, 11135, 11145, 11155, 11166, 11176, 11186, 11197, 11207, 11217, 11228, 11238, 11248, 11258, 11269, 11279, 11289, 11300, 11310, 11320, 11331, 11341, 11351, 11362, 11372, 11382, 11393, 11403, 11413, 11424, 11434, 11444, 11455, 11465, 11475, 11486, 11496, 11506, 11517, 11527, 11537, 11548, 11558, 11568, 11579, 11589, 11599, 11610, 11620, 11630, 11641, 11651, 11661, 11672, 11682, 11692, 11703, 11713, 11724, 11734, 11744, 11755, 11765, 11775, 11786, 11796, 11806, 11817, 11827, 11838, 11848, 11858, 11869, 11879, 11889, 11900, 11910, 11921, 11931, 11941, 11952, 11962, 11972, 11983, 11993, 12004, 12014, 12024, 12035, 12045, 12056, 12066, 12076, 12087, 12097, 12108, 12118, 12128, 12139, 12149, 12160, 12170, 12180, 12191, 12201, 12212, 12222, 12232, 12243, 12253, 12264, 12274, 12285, 12295, 12305, 12316, 12326, 12337, 12347, 12358, 12368, 12378, 12389, 12399, 12410, 12420, 12431, 12441, 12451, 12462, 12472, 12483, 12493, 12504, 12514, 12524, 12535, 12545, 12556, 12566, 12577, 12587, 12598, 12608, 12618, 12629, 12639, 12650, 12660, 12671, 12681, 12692, 12702, 12713, 12723, 12734, 12744, 12754, 12765, 12775, 12786, 12796, 12807, 12817, 12828, 12838, 12849, 12859, 12870, 12880, 12891, 12901, 12912, 12922, 12932, 12943, 12953, 12964, 12974, 12985, 12995, 13006, 13016, 13027, 13037, 13048, 13058, 13069, 13079, 13090, 13100, 13111, 13121, 13132, 13142, 13153, 13163, 13174, 13184, 13195, 13205, 13216, 13226, 13237, 13247, 13258, 13268, 13279, 13289, 13300, 13310, 13321, 13332, 13342, 13353, 13363, 13374, 13384, 13395, 13405, 13416, 13426, 13437, 13447, 13458, 13468, 13479, 13489, 13500, 13511, 13521, 13532, 13542, 13553, 13563, 13574, 13584, 13595, 13605, 13616, 13626, 13637, 13648, 13658, 13669, 13679, 13690, 13700, 13711, 13721, 13732, 13743, 13753, 13764, 13774, 13785, 13795, 13806, 13816, 13827, 13838, 13848, 13859, 13869, 13880, 13890, 13901, 13912, 13922, 13933, 13943, 13954, 13964, 13975, 13986, 13996, 14007, 14017, 14028, 14039, 14049, 14060, 14070, 14081, 14091, 14102, 14113, 14123, 14134, 14144, 14155, 14166, 14176, 14187, 14197, 14208, 14219, 14229, 14240, 14250, 14261, 14272, 14282, 14293, 14303, 14314, 14325, 14335, 14346, 14357, 14367, 14378, 14388, 14399, 14410, 14420, 14431, 14441, 14452, 14463, 14473, 14484, 14495, 14505, 14516, 14526, 14537, 14548, 14558, 14569, 14580, 14590, 14601, 14611, 14622, 14633, 14643, 14654, 14665, 14675, 14686, 14697, 14707, 14718, 14729, 14739, 14750, 14760, 14771, 14782, 14792, 14803, 14814, 14824, 14835, 14846, 14856, 14867, 14878, 14888, 14899, 14910, 14920, 14931, 14942, 14952, 14963, 14974, 14984, 14995, 15006, 15016, 15027, 15038, 15048, 15059, 15070, 15080, 15091, 15102, 15112, 15123, 15134, 15144, 15155, 15166, 15176, 15187, 15198, 15208, 15219, 15230, 15240, 15251, 15262, 15273, 15283, 15294, 15305, 15315, 15326, 15337, 15347, 15358, 15369, 15379, 15390, 15401, 15412, 15422, 15433, 15444, 15454, 15465, 15476, 15486, 15497, 15508, 15519, 15529, 15540, 15551, 15561, 15572, 15583, 15594, 15604, 15615, 15626, 15636, 15647, 15658, 15669, 15679, 15690, 15701, 15712, 15722, 15733, 15744, 15754, 15765, 15776, 15787, 15797, 15808, 15819, 15830, 15840, 15851, 15862, 15872, 15883, 15894, 15905, 15915, 15926, 15937, 15948, 15958, 15969, 15980, 15991, 16001, 16012, 16023, 16034, 16044, 16055, 16066, 16077, 16087, 16098, 16109, 16120, 16130, 16141, 16152, 16163, 16173, 16184, 16195, 16206, 16217, 16227, 16238, 16249, 16260, 16270, 16281, 16292, 16303, 16313, 16324, 16335, 16346, 16357, 16367, 16378, 16389, 16400, 16410, 16421, 16432, 16443, 16454, 16464, 16475, 16486, 16497, 16507, 16518, 16529, 16540, 16551, 16561, 16572, 16583, 16594, 16605, 16615, 16626, 16637, 16648, 16659, 16669, 16680, 16691, 16702, 16713, 16723, 16734, 16745, 16756, 16767, 16777, 16788, 16799, 16810, 16821, 16831, 16842, 16853, 16864, 16875, 16886, 16896, 16907, 16918, 16929, 16940, 16950, 16961, 16972, 16983, 16994, 17005, 17015, 17026, 17037, 17048, 17059, 17069, 17080, 17091, 17102, 17113, 17124, 17134, 17145, 17156, 17167, 17178, 17189, 17199, 17210, 17221, 17232, 17243, 17254, 17265, 17275, 17286, 17297, 17308, 17319, 17330, 17340, 17351, 17362, 17373, 17384, 17395, 17406, 17416, 17427, 17438, 17449, 17460, 17471, 17482, 17492, 17503, 17514, 17525, 17536, 17547, 17558, 17568, 17579, 17590, 17601, 17612, 17623, 17634, 17644, 17655, 17666, 17677, 17688, 17699, 17710, 17721, 17731, 17742, 17753, 17764, 17775, 17786, 17797, 17808, 17819, 17829, 17840, 17851, 17862, 17873, 17884, 17895, 17906, 17916, 17927, 17938, 17949, 17960, 17971, 17982, 17993, 18004, 18015, 18025, 18036, 18047, 18058, 18069, 18080, 18091, 18102, 18113, 18124, 18134, 18145, 18156, 18167, 18178, 18189, 18200, 18211, 18222, 18233, 18243, 18254, 18265, 18276, 18287, 18298, 18309, 18320, 18331, 18342, 18353, 18364, 18374, 18385, 18396, 18407, 18418, 18429, 18440, 18451, 18462, 18473, 18484, 18495, 18506, 18517, 18527, 18538, 18549, 18560, 18571, 18582, 18593, 18604, 18615, 18626, 18637, 18648, 18659, 18670, 18681, 18691, 18702, 18713, 18724, 18735, 18746, 18757, 18768, 18779, 18790, 18801, 18812, 18823, 18834, 18845, 18856, 18867, 18878, 18889, 18900, 18910, 18921, 18932, 18943, 18954, 18965, 18976, 18987, 18998, 19009, 19020, 19031, 19042, 19053, 19064, 19075, 19086, 19097, 19108, 19119, 19130, 19141, 19152, 19163, 19174, 19185, 19196, 19207, 19218, 19229, 19240, 19250, 19261, 19272, 19283, 19294, 19305, 19316, 19327, 19338, 19349, 19360, 19371, 19382, 19393, 19404, 19415, 19426, 19437, 19448, 19459, 19470, 19481, 19492, 19503, 19514, 19525, 19536, 19547, 19558, 19569, 19580, 19591, 19602, 19613, 19624, 19635, 19646, 19657, 19668, 19679, 19690, 19701, 19712, 19723, 19734, 19745, 19756, 19767, 19778, 19789, 19800, 19811, 19822, 19833, 19844, 19855, 19866, 19877, 19888, 19899, 19911, 19922, 19933, 19944, 19955, 19966, 19977, 19988, 19999, 20010, 20021, 20032, 20043, 20054, 20065, 20076, 20087, 20098, 20109, 20120, 20131, 20142, 20153, 20164, 20175, 20186, 20197, 20208, 20219, 20230, 20241, 20253, 20264, 20275, 20286, 20297, 20308, 20319, 20330, 20341, 20352, 20363, 20374, 20385, 20396, 20407, 20418, 20429, 20440, 20451, 20462, 20473, 20485, 20496, 20507, 20518, 20529, 20540, 20551, 20562, 20573, 20584, 20595, 20606, 20617, 20628, 20639, 20650, 20662, 20673, 20684, 20695, 20706, 20717, 20728, 20739, 20750, 20761, 20772, 20783, 20794, 20805, 20817, 20828, 20839, 20850, 20861, 20872, 20883, 20894, 20905, 20916, 20927, 20938, 20950, 20961, 20972, 20983, 20994, 21005, 21016, 21027, 21038, 21049, 21060, 21072, 21083, 21094, 21105, 21116, 21127, 21138, 21149, 21160, 21171, 21182, 21194, 21205, 21216, 21227, 21238, 21249, 21260, 21271, 21282, 21293, 21305, 21316, 21327, 21338, 21349, 21360, 21371, 21382, 21393, 21405, 21416, 21427, 21438, 21449, 21460, 21471, 21482, 21494, 21505, 21516, 21527, 21538, 21549, 21560, 21571, 21582, 21594, 21605, 21616, 21627, 21638, 21649, 21660, 21671, 21683, 21694, 21705, 21716, 21727, 21738, 21749, 21761, 21772, 21783, 21794, 21805, 21816, 21827, 21838, 21850, 21861, 21872, 21883, 21894, 21905, 21916, 21928, 21939, 21950, 21961, 21972, 21983, 21994, 22006, 22017, 22028, 22039, 22050, 22061, 22072, 22084, 22095, 22106, 22117, 22128, 22139, 22151, 22162, 22173, 22184, 22195, 22206, 22217, 22229, 22240, 22251, 22262, 22273, 22284, 22296, 22307, 22318, 22329, 22340, 22351, 22363, 22374, 22385, 22396, 22407, 22418, 22430, 22441, 22452, 22463, 22474, 22485, 22497, 22508, 22519, 22530, 22541, 22553, 22564, 22575, 22586, 22597, 22608, 22620, 22631, 22642, 22653, 22664, 22676, 22687, 22698, 22709, 22720, 22731, 22743, 22754, 22765, 22776, 22787, 22799, 22810, 22821, 22832, 22843, 22855, 22866, 22877, 22888, 22899, 22911, 22922, 22933, 22944, 22955, 22967, 22978, 22989, 23000, 23011, 23023, 23034, 23045, 23056, 23067, 23079, 23090, 23101, 23112, 23123, 23135, 23146, 23157, 23168, 23179, 23191, 23202, 23213, 23224, 23236, 23247, 23258, 23269, 23280, 23292, 23303, 23314, 23325, 23336, 23348, 23359, 23370, 23381, 23393, 23404, 23415, 23426, 23437, 23449, 23460, 23471, 23482, 23494, 23505, 23516, 23527, 23539, 23550, 23561, 23572, 23583, 23595, 23606, 23617, 23628, 23640, 23651, 23662, 23673, 23685, 23696, 23707, 23718, 23730, 23741, 23752, 23763, 23775, 23786, 23797, 23808, 23820, 23831, 23842, 23853, 23865, 23876, 23887, 23898, 23910, 23921, 23932, 23943, 23955, 23966, 23977, 23988, 24000, 24011, 24022, 24033, 24045, 24056, 24067, 24078, 24090, 24101, 24112, 24123, 24135, 24146, 24157, 24168, 24180, 24191, 24202, 24214, 24225, 24236, 24247, 24259, 24270, 24281, 24292, 24304, 24315, 24326, 24337, 24349, 24360, 24371, 24383, 24394, 24405, 24416, 24428, 24439, 24450, 24462, 24473, 24484, 24495, 24507, 24518, 24529, 24541, 24552, 24563, 24574, 24586, 24597, 24608, 24620, 24631, 24642, 24653, 24665, 24676, 24687, 24699, 24710, 24721, 24732, 24744, 24755, 24766, 24778, 24789, 24800, 24811, 24823, 24834, 24845, 24857, 24868, 24879, 24891, 24902, 24913, 24924, 24936, 24947, 24958, 24970, 24981, 24992, 25004, 25015, 25026, 25038, 25049, 25060, 25071, 25083, 25094, 25105, 25117, 25128, 25139, 25151, 25162, 25173, 25185, 25196, 25207, 25219, 25230, 25241, 25253, 25264, 25275, 25287, 25298, 25309, 25320, 25332, 25343, 25354, 25366, 25377, 25388, 25400, 25411, 25422, 25434, 25445, 25456, 25468, 25479, 25490, 25502, 25513, 25524, 25536, 25547, 25558, 25570, 25581, 25592, 25604, 25615, 25626, 25638, 25649, 25660, 25672, 25683, 25694, 25706, 25717, 25728, 25740, 25751, 25763, 25774, 25785, 25797, 25808, 25819, 25831, 25842, 25853, 25865, 25876, 25887, 25899, 25910, 25921, 25933, 25944, 25955, 25967, 25978, 25990, 26001, 26012, 26024, 26035, 26046, 26058, 26069, 26080, 26092, 26103, 26114, 26126, 26137, 26149, 26160, 26171, 26183, 26194, 26205, 26217, 26228, 26239, 26251, 26262, 26274, 26285, 26296, 26308, 26319, 26330, 26342, 26353, 26365, 26376, 26387, 26399, 26410, 26421, 26433, 26444, 26456, 26467, 26478, 26490, 26501, 26512, 26524, 26535, 26547, 26558, 26569, 26581, 26592, 26604, 26615, 26626, 26638, 26649, 26660, 26672, 26683, 26695, 26706, 26717, 26729, 26740, 26752, 26763, 26774, 26786, 26797, 26809, 26820, 26831, 26843, 26854, 26866, 26877, 26888, 26900, 26911, 26923, 26934, 26945, 26957, 26968, 26980, 26991, 27002, 27014, 27025, 27037, 27048, 27059, 27071, 27082, 27094, 27105, 27116, 27128, 27139, 27151, 27162, 27173, 27185, 27196, 27208, 27219, 27231, 27242, 27253, 27265, 27276, 27288, 27299, 27310, 27322, 27333, 27345, 27356, 27368, 27379, 27390, 27402, 27413, 27425, 27436, 27448, 27459, 27470, 27482, 27493, 27505, 27516, 27528, 27539, 27550, 27562, 27573, 27585, 27596, 27608, 27619, 27630, 27642, 27653, 27665, 27676, 27688, 27699, 27710, 27722, 27733, 27745, 27756, 27768, 27779, 27791, 27802, 27813, 27825, 27836, 27848, 27859, 27871, 27882, 27894, 27905, 27916, 27928, 27939, 27951, 27962, 27974, 27985, 27997, 28008, 28019, 28031, 28042, 28054, 28065, 28077, 28088, 28100, 28111, 28123, 28134, 28145, 28157, 28168, 28180, 28191, 28203, 28214, 28226, 28237, 28249, 28260, 28271, 28283, 28294, 28306, 28317, 28329, 28340, 28352, 28363, 28375, 28386, 28398, 28409, 28421, 28432, 28443, 28455, 28466, 28478, 28489, 28501, 28512, 28524, 28535, 28547, 28558, 28570, 28581, 28593, 28604, 28616, 28627, 28639, 28650, 28662, 28673, 28684, 28696, 28707, 28719, 28730, 28742, 28753, 28765, 28776, 28788, 28799, 28811, 28822, 28834, 28845, 28857, 28868, 28880, 28891, 28903, 28914, 28926, 28937, 28949, 28960, 28972, 28983, 28995, 29006, 29018, 29029, 29041, 29052, 29064, 29075, 29087, 29098, 29110, 29121, 29133, 29144, 29156, 29167, 29179, 29190, 29202, 29213, 29225, 29236, 29248, 29259, 29271, 29282, 29294, 29305, 29317, 29328, 29340, 29351, 29363, 29374, 29386, 29397, 29409, 29420, 29432, 29443, 29455, 29466, 29478, 29489, 29501, 29512, 29524, 29535, 29547, 29558, 29570, 29581, 29593, 29605, 29616, 29628, 29639, 29651, 29662, 29674, 29685, 29697, 29708, 29720, 29731, 29743, 29754, 29766, 29777, 29789, 29800, 29812, 29823, 29835, 29847, 29858, 29870, 29881, 29893, 29904, 29916, 29927, 29939, 29950, 29962, 29973, 29985, 29996, 30008, 30020, 30031, 30043, 30054, 30066, 30077, 30089, 30100, 30112, 30123, 30135, 30147, 30158, 30170, 30181, 30193, 30204, 30216, 30227, 30239, 30250, 30262, 30274, 30285, 30297, 30308, 30320, 30331, 30343, 30354, 30366, 30377, 30389, 30401, 30412, 30424, 30435, 30447, 30458, 30470, 30481, 30493, 30505, 30516, 30528, 30539, 30551, 30562, 30574, 30585, 30597, 30609, 30620, 30632, 30643, 30655, 30666, 30678, 30690, 30701, 30713, 30724, 30736, 30747, 30759, 30771, 30782, 30794, 30805, 30817, 30828, 30840, 30852, 30863, 30875, 30886, 30898, 30909, 30921, 30933, 30944, 30956, 30967, 30979, 30990, 31002, 31014, 31025, 31037, 31048, 31060, 31071, 31083, 31095, 31106, 31118, 31129, 31141, 31153, 31164, 31176, 31187, 31199, 31211, 31222, 31234, 31245, 31257, 31268, 31280, 31292, 31303, 31315, 31326, 31338, 31350, 31361, 31373, 31384, 31396, 31408, 31419, 31431, 31442, 31454, 31466, 31477, 31489, 31500, 31512, 31524, 31535, 31547, 31558, 31570, 31582, 31593, 31605, 31616, 31628, 31640, 31651, 31663, 31674, 31686, 31698, 31709, 31721, 31732, 31744, 31756, 31767, 31779, 31790, 31802, 31814, 31825, 31837, 31849, 31860, 31872, 31883, 31895, 31907, 31918, 31930, 31941, 31953, 31965, 31976, 31988, 32000, 32011, 32023, 32034, 32046, 32058, 32069, 32081, 32092, 32104, 32116, 32127, 32139, 32151, 32162, 32174, 32185, 32197, 32209, 32220, 32232, 32244, 32255, 32267, 32278, 32290, 32302, 32313, 32325, 32337, 32348, 32360, 32372, 32383, 32395, 32406, 32418, 32430, 32441, 32453, 32465, 32476, 32488, 32500, 32511, 32523, 32534, 32546, 32558, 32569, 32581, 32593, 32604, 32616, 32628, 32639, 32651, 32663, 32674, 32686, 32697, 32709, 32721, 32732, 32744, 32756, 32767, 32779, 32791, 32802, 32814, 32826, 32837, 32849, 32861, 32872, 32884, 32895, 32907, 32919, 32930, 32942, 32954, 32965, 32977, 32989, 33000, 33012, 33024, 33035, 33047, 33059, 33070, 33082, 33094, 33105, 33117, 33129, 33140, 33152, 33164, 33175, 33187, 33199, 33210, 33222, 33234, 33245, 33257, 33269, 33280, 33292, 33304, 33315, 33327, 33339, 33350, 33362, 33374, 33385, 33397, 33409, 33420, 33432, 33444, 33455, 33467, 33479, 33490, 33502, 33514, 33525, 33537, 33549, 33560, 33572, 33584, 33596, 33607, 33619, 33631, 33642, 33654, 33666, 33677, 33689, 33701, 33712, 33724, 33736, 33747, 33759, 33771, 33782, 33794, 33806, 33818, 33829, 33841, 33853, 33864, 33876, 33888, 33899, 33911, 33923, 33934, 33946, 33958, 33969, 33981, 33993, 34005, 34016, 34028, 34040, 34051, 34063, 34075, 34086, 34098, 34110, 34122, 34133, 34145, 34157, 34168, 34180, 34192, 34203, 34215, 34227, 34239, 34250, 34262, 34274, 34285, 34297, 34309, 34321, 34332, 34344, 34356, 34367, 34379, 34391, 34402, 34414, 34426, 34438, 34449, 34461, 34473, 34484, 34496, 34508, 34520, 34531, 34543, 34555, 34566, 34578, 34590, 34602, 34613, 34625, 34637, 34648, 34660, 34672, 34684, 34695, 34707, 34719, 34731, 34742, 34754, 34766, 34777, 34789, 34801, 34813, 34824, 34836, 34848, 34859, 34871, 34883, 34895, 34906, 34918, 34930, 34942, 34953, 34965, 34977, 34989, 35000, 35012, 35024, 35035, 35047, 35059, 35071, 35082, 35094, 35106, 35118, 35129, 35141, 35153, 35165, 35176, 35188, 35200, 35211, 35223, 35235, 35247, 35258, 35270, 35282, 35294, 35305, 35317, 35329, 35341, 35352, 35364, 35376, 35388, 35399, 35411, 35423, 35435, 35446, 35458, 35470, 35482, 35493, 35505, 35517, 35529, 35540, 35552, 35564, 35576, 35587, 35599, 35611, 35623, 35634, 35646, 35658, 35670, 35681, 35693, 35705, 35717, 35728, 35740, 35752, 35764, 35775, 35787, 35799, 35811, 35823, 35834, 35846, 35858, 35870, 35881, 35893, 35905, 35917, 35928, 35940, 35952, 35964, 35975, 35987, 35999, 36011, 36023, 36034, 36046, 36058, 36070, 36081, 36093, 36105, 36117, 36128, 36140, 36152, 36164, 36176, 36187, 36199, 36211, 36223, 36234, 36246, 36258, 36270, 36282, 36293, 36305, 36317, 36329, 36340, 36352, 36364, 36376, 36388, 36399, 36411, 36423, 36435, 36446, 36458, 36470, 36482, 36494, 36505, 36517, 36529, 36541, 36553, 36564, 36576, 36588, 36600, 36611, 36623, 36635, 36647, 36659, 36670, 36682, 36694, 36706, 36718, 36729, 36741, 36753, 36765, 36777, 36788, 36800, 36812, 36824, 36836, 36847, 36859, 36871, 36883, 36895, 36906, 36918, 36930, 36942, 36954, 36965, 36977, 36989, 37001, 37013, 37024, 37036, 37048, 37060, 37072, 37083, 37095, 37107, 37119, 37131, 37142, 37154, 37166, 37178, 37190, 37201, 37213, 37225, 37237, 37249, 37260, 37272, 37284, 37296, 37308, 37320, 37331, 37343, 37355, 37367, 37379, 37390, 37402, 37414, 37426, 37438, 37449, 37461, 37473, 37485, 37497, 37509, 37520, 37532, 37544, 37556, 37568, 37580, 37591, 37603, 37615, 37627, 37639, 37650, 37662, 37674, 37686, 37698, 37710, 37721, 37733, 37745, 37757, 37769, 37781, 37792, 37804, 37816, 37828, 37840, 37852, 37863, 37875, 37887, 37899, 37911, 37923, 37934, 37946, 37958, 37970, 37982, 37994, 38005, 38017, 38029, 38041, 38053, 38065, 38076, 38088, 38100, 38112, 38124, 38136, 38147, 38159, 38171, 38183, 38195, 38207, 38218, 38230, 38242, 38254, 38266, 38278, 38290, 38301, 38313, 38325, 38337, 38349, 38361, 38372, 38384, 38396, 38408, 38420, 38432, 38444, 38455, 38467, 38479, 38491, 38503, 38515, 38527, 38538, 38550, 38562, 38574, 38586, 38598, 38610, 38621, 38633, 38645, 38657, 38669, 38681, 38693, 38704, 38716, 38728, 38740, 38752, 38764, 38776, 38787, 38799, 38811, 38823, 38835, 38847, 38859, 38870, 38882, 38894, 38906, 38918, 38930, 38942, 38954, 38965, 38977, 38989, 39001, 39013, 39025, 39037, 39048, 39060, 39072, 39084, 39096, 39108, 39120, 39132, 39143, 39155, 39167, 39179, 39191, 39203, 39215, 39227, 39238, 39250, 39262, 39274, 39286, 39298, 39310, 39322, 39333, 39345, 39357, 39369, 39381, 39393, 39405, 39417, 39429, 39440, 39452, 39464, 39476, 39488, 39500, 39512, 39524, 39535, 39547, 39559, 39571, 39583, 39595, 39607, 39619, 39631, 39642, 39654, 39666, 39678, 39690, 39702, 39714, 39726, 39738, 39749, 39761, 39773, 39785, 39797, 39809, 39821, 39833, 39845, 39857, 39868, 39880, 39892, 39904, 39916, 39928, 39940, 39952, 39964, 39976, 39987, 39999, 40011, 40023, 40035, 40047, 40059, 40071, 40083, 40095, 40106, 40118, 40130, 40142, 40154, 40166, 40178, 40190, 40202, 40214, 40226, 40237, 40249, 40261, 40273, 40285, 40297, 40309, 40321, 40333, 40345, 40357, 40368, 40380, 40392, 40404, 40416, 40428, 40440, 40452, 40464, 40476, 40488, 40500, 40511, 40523, 40535, 40547, 40559, 40571, 40583, 40595, 40607, 40619, 40631, 40643, 40654, 40666, 40678, 40690, 40702, 40714, 40726, 40738, 40750, 40762, 40774, 40786, 40798, 40809, 40821, 40833, 40845, 40857, 40869, 40881, 40893, 40905, 40917, 40929, 40941, 40953, 40965, 40976, 40988, 41000, 41012, 41024, 41036, 41048, 41060, 41072, 41084, 41096, 41108, 41120, 41132, 41144, 41155, 41167, 41179, 41191, 41203, 41215, 41227, 41239, 41251, 41263, 41275, 41287, 41299, 41311, 41323, 41335, 41347, 41358, 41370, 41382, 41394, 41406, 41418, 41430, 41442, 41454, 41466, 41478, 41490, 41502, 41514, 41526, 41538, 41550, 41562, 41574, 41585, 41597, 41609, 41621, 41633, 41645, 41657, 41669, 41681, 41693, 41705, 41717, 41729, 41741, 41753, 41765, 41777, 41789, 41801, 41813, 41825, 41837, 41848, 41860, 41872, 41884, 41896, 41908, 41920, 41932, 41944, 41956, 41968, 41980, 41992, 42004, 42016, 42028, 42040, 42052, 42064, 42076, 42088, 42100, 42112, 42124, 42136, 42148, 42159, 42171, 42183, 42195, 42207, 42219, 42231, 42243, 42255, 42267, 42279, 42291, 42303, 42315, 42327, 42339, 42351, 42363, 42375, 42387, 42399, 42411, 42423, 42435, 42447, 42459, 42471, 42483, 42495, 42507, 42519, 42531, 42543, 42555, 42567, 42579, 42591, 42603, 42615, 42627, 42638, 42650, 42662, 42674, 42686, 42698, 42710, 42722, 42734, 42746, 42758, 42770, 42782, 42794, 42806, 42818, 42830, 42842, 42854, 42866, 42878, 42890, 42902, 42914, 42926, 42938, 42950, 42962, 42974, 42986, 42998, 43010, 43022, 43034, 43046, 43058, 43070, 43082, 43094, 43106, 43118, 43130, 43142, 43154, 43166, 43178, 43190, 43202, 43214, 43226, 43238};
  //Calculate the LMP
  lmp = (mcnt < support_size / 2) | (-(mcnt >= support_size / 2) & ((int32_t)logfact[support_size] - (int32_t)logfact[mcnt] - (int32_t)logfact[support_size - mcnt] - (int32_t)support_size));

  return lmp;
}

/*************************************************
* Name:        random_int_range
*
* Description: Sample integer in range [0,max_range]
*
* Arguments:   - keccak_state *state: Preinitialized state to draw random bits from
*              - uint16_t max_range: max range for sampling
**************************************************/
uint16_t random_int_range(keccak_state *state, uint16_t max_range) {
  uint16_t mask, value;
  mask = max_range;

  /* Smallest bit mask >= max_range */
  mask |= mask >> 1;
  mask |= mask >> 2;
  mask |= mask >> 4;
  mask |= mask >> 8;

  /* Apply mask and reject is needed -> random value <= max_range */
  shake256_squeeze((uint8_t*) &value, 2, state);
  while ((value = (value & mask)) > max_range){
    shake256_squeeze((uint8_t*) &value, 2, state);
  }
  return value;
}

/*************************************************
* Name:        random_int_range
*
* Description: Constant time variant of the Fisher-Yates Shuffle
*              Published by Nicolas Sendrier in https://eprint.iacr.org/2021/1631
*              Corresponds to Algorithm 3
*
* Arguments:   - keccak_state *state: Preinitialized state to draw random bits from
*              - uint16_t* array: Pointer to array with shuffled indices
**************************************************/
void shuffle_array_indices(keccak_state *state, uint16_t* array){
  int16_t i, j;
  uint16_t x;
  int mask = 0;
  for(i = N/2-1; i >= 0; i--){
    //Sample integer in [i,N/2-1]
    x = i + random_int_range(state, N/2 -1 - i);
    array[i] = x;
    for (j= i+1; j <= N/2-1; j++){
      // Conditional assignment for array[j] == array[i]
      mask = -(array[j] == array[i]);
      array[j] = (i & mask) | (array[j] & ~mask);
    }
  }
}

/*************************************************
* Name:        expand_key
*
* Description: Expand sk a and b to matrix Gsec
*
* Arguments:   - poly_s_n gsec[N/2]: Expanded sk in array of polynomials with cyclic shift
*              - const poly_s_n_2 *a: Pointer to sk polynomial a
*              - const poly_s_n_2 *b: Pointer to sk polynomial b
**************************************************/
void expand_key(poly_s_n gsec[N/2], const poly_s_n_2 *a, const poly_s_n_2 *b)
{
  size_t i, j;
  size_t rel_addr;
  for(i = 0; i < N/2; i++)
  {
    for(j = 0; j < N/2; j++)
    {
      rel_addr = rel_key_address(i, j);
      gsec[i].coeffs[j] = a->coeffs[rel_addr];
      gsec[i].coeffs[N/2 + j] = b->coeffs[rel_addr];
    }
  }
}

/*************************************************
* Name:        expand_chall
*
* Description: Expand chall from 8 bits in one byte to one bit in one byte to avoid shifting every round
*
* Arguments:   - uint8_t exp_chall[N]: Array with expanded challenge
*              - const uint8_t *mhash: Challenge to be expanded
**************************************************/
void expand_chall(uint8_t exp_chall[N], const uint8_t *mhash)
{
  size_t i;
  size_t curr_chall_index, curr_chall_bit;
  for (i = 0; i < N; i++)
  {
    curr_chall_index = i >> 3;
    curr_chall_bit = i & 7;
    //Extract relevant challenge bits
    exp_chall[i] = ((mhash[curr_chall_index] >> curr_chall_bit) & 1);
  }
}
