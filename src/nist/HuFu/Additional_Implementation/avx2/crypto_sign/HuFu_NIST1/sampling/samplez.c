/*
 * Floating-point operations.
 *
 * ==========================(LICENSE BEGIN)============================
 *
 * Copyright (c) 2017-2019  Falcon Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ===========================(LICENSE END)=============================
 *
 * @author   Thomas Pornin <thomas.pornin@nccgroup.com>
 */
 
#include <stdint.h>
#include <math.h>
#include <stddef.h>

#include "samplez.h"
#include "../random/random.h"
#include "../params.h"
#include "fpr.h"

#include <time.h>
#include <stdio.h>

#define TABLE_SIZE 12

uint64_t CDT[TABLE_SIZE] = {8434467551140579328LLU, 14834991278396909568LLU,
                            17631969161567469056LLU, 18335816111301163904LLU,
                            18437812105058141248LLU, 18446323557459330173LLU,
                            18446732573100420169LLU, 18446743891677628031LLU,
                            18446744072045607188LLU, 18446744073700777628LLU,
                            18446744073709524277LLU, 18446744073709550894LLU};


int base_sampler(prng *rng) {
//    uint64_t r = get64();
    uint64_t r;
    prng_get_bytes(rng, (uint8_t *)&r, 8);
    int res = 0;
    for (int i = 0; i < TABLE_SIZE; ++i) res += (r >= CDT[i]);
    return res;
}


int16_t samplez(double u,prng *rng) {
  int16_t z0, b, z, uf;
  double neg_x;
  uint32_t w;
  uint64_t neg_exp;
  uint8_t entropy;
  uf = floor(u);
  while (1) {
    // entropy = get8();

    // randombytes((uint8_t *)&entropy, 1);
    prng_get_bytes(rng, (uint8_t *)&entropy, 1);
    for (int i = 0; i < 8; ++i) {
     z0 = base_sampler_RCDT(rng);
      // z0 = base_sampler(rng);

      b = (entropy >> i) & 1;
      z = (2 * b - 1) * z0 + b + uf;  // add floor u here because we subtract u at the next step
      neg_x = (((double)(z - u) * (z - u))-(double)(z0 * z0)) / (2 * PARAM_R * PARAM_R);
//        neg_x = (((double)(z*z - 2*z*u + u*u))-(double)(z0 * z0)) / (2 * PARAM_R * PARAM_R);
      
      neg_exp = ExpM63(FPR(neg_x));
      
      // randombytes((uint8_t *)&r64, 8);
      ;
      i = 64;
	  do {
		i -= 8;
        prng_get_bytes(rng, (uint8_t *)&entropy, 1);
		w = entropy - ((uint32_t)(neg_exp >> i) & 0xFF);
	  } while (!w && i > 0);
      if ((int)(w >> 31)){
        return z;
      }
    }
  }
}


/*
  Sampling of continue centers by using RCDT
*/


  static const uint32_t dist[] = 
  {
    9012524u, 15192425u, 15853829u,
    3172061u, 13917703u, 5737857u,
    686545u, 12076027u, 6719412u,
    88089u, 15395855u, 10985591u,
    6564u, 6030263u, 1110348u,
    280u, 14404062u, 4382442u,
    6u, 14348844u, 14221742u,
    0u, 1595710u, 8739360u,
    0u, 12559u, 592817u,
    0u, 56u, 138767u,
    0u, 0u, 2372903u,
    0u, 0u, 3391u,
    0u, 0u, 2u
  };

int16_t base_sampler_RCDT(prng *rng){
  uint32_t v0, v1, v2, hi;
  hi = 0;
  uint64_t lo;
	size_t u;
	int z;

	/*
	 * Get a random 72-bit value, into three 24-bit limbs v0..v2.
	 */
	// lo = get64();
	// hi = get8();

 
  // randombytes((uint8_t *)&lo, 8);
  // randombytes((uint8_t *)&hi, 1);
  prng_get_bytes(rng, (uint8_t *)&lo, 8);
      prng_get_bytes(rng, (uint8_t *)&hi, 1);
  v0 = (uint32_t)lo & 0xFFFFFF;
  v1 = (uint32_t)(lo >> 24) & 0xFFFFFF;
	v2 = (uint32_t)(lo >> 48) | (hi << 16);
  
  /*
	 * Sampled value is z, such that v0..v2 is lower than the first
	 * z elements of the table.
	 */
	z = 0;
	for (u = 0; u < (sizeof dist) / sizeof(dist[0]); u += 3) {
		uint32_t w0, w1, w2, cc;

		w0 = dist[u + 2];
		w1 = dist[u + 1];
		w2 = dist[u + 0];
		cc = (v0 - w0) >> 31;
		cc = (v1 - w1 - cc) >> 31;
		cc = (v2 - w2 - cc) >> 31;
		z += (int)cc;
  }

  return z;
  
}

static const uint32_t dist0[9][78] = {
    {
    16777215u, 16777215u, 16777215u,
    16777215u, 16777215u, 16775011u,
    16777215u, 16777215u, 15233552u,
    16777215u, 16777179u, 9470246u,
    16777215u, 16769045u, 14522865u,
    16777215u, 15739145u, 1332846u,
    16777211u, 9066269u, 6939571u,
    16777033u, 4877764u, 16146876u,
    16772945u, 10608537u, 11482579u,
    16719910u, 2157349u, 15627080u,
    16330591u, 11187431u, 5168729u,
    14713668u, 1162033u, 6316591u,
    10914222u, 12454635u, 9807704u,
    5862993u, 4322580u, 6969512u,
    2063547u, 15615182u, 10460625u,
    446624u, 5589784u, 11608487u,
    57305u, 14619866u, 1150136u,
    4270u, 6168678u, 5294637u,
    182u, 11899451u, 630340u,
    4u, 7710946u, 9837645u,
    0u, 1038070u, 15444370u,
    0u, 8170u, 2254351u,
    0u, 36u, 7306970u,
    0u, 0u, 1543664u,
    0u, 0u, 2205u,
    0u, 0u, 1u
    },
    {
    16777215u, 16777215u, 16777215u,
    16777215u, 16777215u, 16775779u,
    16777215u, 16777215u, 15734918u,
    16777215u, 16777190u, 8519352u,
    16777215u, 16771293u, 13856u,
    16777215u, 15997481u, 2513950u,
    16777212u, 8884624u, 4508330u,
    16777068u, 12078839u, 6687720u,
    16773651u, 5683901u, 11687263u,
    16727705u, 10250903u, 10929223u,
    16378208u, 15703768u, 14490080u,
    14874072u, 12477629u, 13387724u,
    11211574u, 1456957u, 7088950u,
    6165960u, 8535982u, 839654u,
    2233207u, 4266951u, 2499711u,
    498901u, 11986748u, 6858912u,
    66187u, 15716961u, 4844771u,
    5104u, 11760568u, 13330078u,
    226u, 2761788u, 7939383u,
    5u, 12047547u, 8851391u,
    0u, 1378939u, 9602935u,
    0u, 11244u, 13954839u,
    0u, 51u, 16123699u,
    0u, 0u, 2281119u,
    0u, 0u, 3377u,
    0u, 0u, 2u

  },
  {
    16777216u, 0u, 0u,
    16777215u, 16777215u, 16776282u,
    16777215u, 16777215u, 16075008u,
    16777215u, 16777198u, 3420825u,
    16777215u, 16772931u, 10019250u,
    16777215u, 16192823u, 3671359u,
    16777213u, 5121881u, 11581530u,
    16777097u, 9048345u, 1478444u,
    16774246u, 14686656u, 5022514u,
    16734531u, 10422121u, 13873334u,
    16421480u, 13122408u, 1342376u,
    15025374u, 2116586u, 11478387u,
    11502732u, 199963u, 13103970u,
    6473928u, 12068169u, 16138360u,
    2412236u, 14359899u, 12980950u,
    556161u, 16040368u, 10443848u,
    76284u, 10717728u, 11789395u,
    6088u, 14022374u, 14339575u,
    279u, 5742655u, 16086714u,
    7u, 5294054u, 11750430u,
    0u, 1827686u, 10410941u,
    0u, 15442u, 5669306u,
    0u, 73u, 15731965u,
    0u, 0u, 3363395u,
    0u, 0u, 5161u,
    0u, 0u, 4u

  },
  {
    16777216u, 0u, 0u,
    16777215u, 16777215u, 16776611u,
    16777215u, 16777215u, 16305182u,
    16777215u, 16777203u, 10135410u,
    16777215u, 16774123u, 12536703u,
    16777215u, 16340196u, 12544412u,
    16777213u, 15304752u, 2987106u,
    16777120u, 15575234u, 1968575u,
    16774748u, 4787791u, 7259276u,
    16740494u, 15322764u, 4474492u,
    16460712u, 7759407u, 2088210u,
    15167756u, 4648416u, 3058045u,
    11787159u, 11933600u, 5835273u,
    6786249u, 3272430u, 4245928u,
    2600713u, 13079649u, 9005847u,
    618734u, 4683791u, 16134114u,
    87735u, 8947395u, 8925252u,
    7246u, 16719176u, 11997485u,
    344u, 4503102u, 9130208u,
    9u, 5681958u, 3044503u,
    0u, 2417111u, 8148392u,
    0u, 21159u, 11787304u,
    0u, 104u, 16371337u,
    0u, 0u, 4948144u,
    0u, 0u, 7869u,
    0u, 0u, 7u

  },
  {
    16777216u, 0u, 0u,
    16777215u, 16777215u, 16776824u,
    16777215u, 16777215u, 16460612u,
    16777215u, 16777207u, 6455988u,
    16777215u, 16774989u, 2112471u,
    16777215u, 16451128u, 10488461u,
    16777214u, 6476734u, 102573u,
    16777139u, 14543202u, 8921074u,
    16775169u, 7973744u, 14815034u,
    16745692u, 8189814u, 649095u,
    16496198u, 10729516u, 3309815u,
    15301432u, 13605636u, 8529274u,
    12064363u, 15299019u, 13921129u,
    7102243u, 9426242u, 528518u,
    2798673u, 14100379u, 12355221u,
    686952u, 9672592u, 118565u,
    100692u, 3354292u, 12123958u,
    8606u, 13729655u, 8241237u,
    423u, 5987695u, 3524427u,
    11u, 15016101u, 4328906u,
    0u, 3189556u, 2664037u,
    0u, 28929u, 10235654u,
    0u, 148u, 11955907u,
    0u, 0u, 7263424u,
    0u, 0u, 11970u,
    0u, 0u, 11u
  },
  {
    16777216u, 0u, 0u,
    16777215u, 16777215u, 16776963u,
    16777215u, 16777215u, 16565334u,
    16777215u, 16777210u, 431050u,
    16777215u, 16775615u, 14782869u,
    16777215u, 16534440u, 7518157u,
    16777214u, 12668094u, 11574123u,
    16777155u, 2794598u, 7322704u,
    16775522u, 7823380u, 2944073u,
    16750212u, 3442583u, 13732427u,
    16528222u, 7064732u, 14157513u,
    15426643u, 11162773u, 152241u,
    12333897u, 3899757u, 7500282u,
    7421208u, 2852132u, 14942011u,
    3006109u, 3952054u, 5078665u,
    761153u, 6675749u, 15883187u,
    115318u, 11991252u, 4430286u,
    10199u, 12375809u, 13360834u,
    519u, 7989154u, 653598u,
    15u, 1976374u, 5873772u,
    0u, 4199548u, 11784434u,
    0u, 39465u, 164953u,
    0u, 210u, 3429641u,
    0u, 0u, 10638369u,
    0u, 0u, 18166u,
    0u, 0u, 17u
  },
  {
    16777215u, 16777215u, 16777053u,
    16777215u, 16777215u, 16635733u,
    16777215u, 16777211u, 14533128u,
    16777215u, 16776068u, 13176720u,
    16777215u, 16596867u, 2818899u,
    16777215u, 702139u, 2158111u,
    16777167u, 8361981u, 11462669u,
    16775817u, 10427652u, 6087842u,
    16754133u, 6628640u, 8505703u,
    16557054u, 9982058u, 10422601u,
    15543651u, 13920268u, 12511946u,
    12595360u, 976548u, 2123600u,
    7742418u, 13228792u, 4397967u,
    3222966u, 7593824u, 6273837u,
    841673u, 12197092u, 417005u,
    131792u, 1022564u, 12422311u,
    12061u, 6921901u, 10390765u,
    636u, 433892u, 15282340u,
    19u, 2876280u, 6165299u,
    0u, 5517139u, 9715242u,
    0u, 53717u, 14145998u,
    0u, 296u, 7776896u,
    0u, 0u, 15546888u,
    0u, 0u, 27509u,
    0u, 0u, 27u
  },
  {
    16777216u, 0u, 0u,
    16777215u, 16777215u, 16777112u,
    16777215u, 16777215u, 16682952u,
    16777215u, 16777213u, 2451297u,
    16777215u, 16776395u, 5400393u,
    16777215u, 16643538u, 4805989u,
    16777215u, 4432422u, 9776448u,
    16777177u, 6968663u, 4142559u,
    16776063u, 14264369u, 4909544u,
    16757527u, 7823048u, 1595304u,
    16582953u, 799962u, 1788418u,
    15652740u, 1238522u, 3476302u,
    12848401u, 15485017u, 9852662u,
    8065135u, 9010397u, 4506077u,
    3449144u, 8647345u, 7184768u,
    928848u, 9370405u, 3497225u,
    150302u, 7802978u, 13797364u,
    14232u, 2903892u, 14449980u,
    777u, 408185u, 5570114u,
    24u, 4337109u, 5205347u,
    0u, 7232101u, 2056411u,
    0u, 72956u, 2049618u,
    0u, 417u, 3225336u,
    0u, 1u, 5892529u,
    0u, 0u, 41566u,
    0u, 0u, 43u
  },
  {
    16777216u, 0u, 0u,
    16777215u, 16777215u, 16777149u,
    16777215u, 16777215u, 16714551u,
    16777215u, 16777214u, 571856u,
    16777215u, 16776630u, 3641516u,
    16777215u, 16678351u, 2749011u,
    16777215u, 7318016u, 12418468u,
    16777185u, 6242755u, 14064432u,
    16776268u, 13271651u, 8965282u,
    16760458u, 8383848u, 4298097u,
    16606162u, 5963861u, 1213664u,
    15754207u, 11955561u, 14308639u,
    13092722u, 6657257u, 16408895u,
    8388608u, 0u, 0u,
    3684493u, 10119958u, 368321u,
    1023008u, 4821654u, 2468577u,
    171053u, 10813354u, 15563552u,
    16757u, 8393367u, 12479119u,
    947u, 3505564u, 7811934u,
    30u, 10534460u, 2712784u,
    0u, 9459199u, 4358748u,
    0u, 98864u, 14028205u,
    0u, 585u, 13135700u,
    0u, 1u, 16205360u,
    0u, 0u, 62665u,
    0u, 0u, 67u
  }
  };

/*
  For sampling of discrete centers {0,1/16,...,15/16} by using 9 differernt RCDT
*/
int16_t base_sampler_RCDT_discrete_center(int16_t center,prng *rng){
  uint32_t v0, v1, v2, hi;
  hi = 0;
  uint64_t lo;
	size_t u;
	int z;

	/*
	 * Get a random 72-bit value, into three 24-bit limbs v0..v2.
	 */
	// lo = get64();
	// hi = get8();
  // randombytes((uint8_t *)&lo, 8);
  // randombytes((uint8_t *)&hi, 1);
  prng_get_bytes(rng, (uint8_t *)&lo, 8);
  prng_get_bytes(rng, (uint8_t *)&hi, 1);
  v0 = (uint32_t)lo & 0xFFFFFF;
	v1 = (uint32_t)(lo >> 24) & 0xFFFFFF;
	v2 = (uint32_t)(lo >> 48) | (hi << 16);

	/*
	 * Sampled value is z, such that v0..v2 is lower than the first
	 * z elements of the table.
	 */
  int c = center;
  c = (c > 8) * (16 - 2 * c) + c;
  z = 0;
    for (u = 0; u < (sizeof dist0[0]) / sizeof(dist0[0][0]); u += 3) {
        uint32_t w0, w1, w2, cc;

        w0 = dist0[c][u + 2];
        w1 = dist0[c][u + 1];
        w2 = dist0[c][u + 0];
        cc = (v0 - w0) >> 31;
        cc = (v1 - w1 - cc) >> 31;
        cc = (v2 - w2 - cc) >> 31;
        z += (int)cc;
    }
    return (center > 8) * (27 - 2 * z) + z - 13;

}