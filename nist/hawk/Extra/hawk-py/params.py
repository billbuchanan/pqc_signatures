# This files contains the parameters and CDT tables for Hawk-(256,512,1024)

Hawk256_T0 = [
    0x26B871FBD58485D45050,
    0x07C054114F1DC2FA7AC9,
    0x00A242F74ADDA0B5AE61,
    0x0005252E2152AB5D758B,
    0x00000FDE62196C1718FC,
    0x000000127325DDF8CEBA,
    0x0000000008100822C548,
    0x00000000000152A6E9AE,
    0x0000000000000014DA4A,
    0x0000000000000000007B,
]

Hawk512_T0 = [
    0x2C058C27920A04F8F267,
    0x0E9A1C4FF17C204AA058,
    0x02DBDE63263BE0098FFD,
    0x005156AEDFB0876A3BD8,
    0x0005061E21D588CC61CC,
    0x00002BA568D92EEC18E7,
    0x000000CF0F8687D3B009,
    0x0000000216A0C344EB45,
    0x0000000002EDF0B98A84,
    0x0000000000023AF3B2E7,
    0x00000000000000EBCC6A,
    0x000000000000000034CF,
    0x00000000000000000006,
]

Hawk1024_T0 = [
    0x2C583AAA2EB76504E560,
    0x0F1D70E1C03E49BB683E,
    0x031955CDA662EF2D1C48,
    0x005E31E874B355421BB7,
    0x000657C0676C029895A7,
    0x00003D4D67696E51F820,
    0x0000014A1A8A93F20738,
    0x00000003DAF47E8DFB21,
    0x0000000006634617B3FF,
    0x000000000005DBEFB646,
    0x00000000000002F93038,
    0x0000000000000000D5A7,
    0x00000000000000000021,
]

Hawk256_T1 = [
    0x13459408A4B181C718B1,
    0x027D614569CC54722DC9,
    0x0020951C5CDCBAFF49A3,
    0x0000A3460C30AC398322,
    0x000001355A8330C44097,
    0x00000000DC8DE401FD12,
    0x00000000003B0FFB28F0,
    0x00000000000005EFCD99,
    0x00000000000000003953,
    0x00000000000000000000,
]

Hawk512_T1 = [
    0x1AFCBC689D9213449DC9,
    0x06EBFB908C81FCE3524F,
    0x01064EBEFD8FF4F07378,
    0x0015C628BC6B23887196,
    0x0000FF769211F07B326F,
    0x00000668F461693DFF8F,
    0x0000001670DB65964485,
    0x000000002AB6E11C2552,
    0x00000000002C253C7E81,
    0x00000000000018C14ABF,
    0x0000000000000007876E,
    0x0000000000000000013D,
    0x00000000000000000000,
]

Hawk1024_T1 = [
    0x1B7F01AE2B17728DF2DE,
    0x07506A00B82C69624C93,
    0x01252685DB30348656A4,
    0x001A430192770E205503,
    0x00015353BD4091AA96DB,
    0x000009915A53D8667BEE,
    0x00000026670030160D5F,
    0x00000000557CD1C5F797,
    0x00000000006965E15B13,
    0x00000000000047E9AB38,
    0x000000000000001B2445,
    0x000000000000000005AA,
    0x00000000000000000000,
]


HAWK_256_PARAMS = {
    "lenpriv": 96,
    "lenpub": 450,
    "lensig": 249,
    "n": 256,
    "qs": 2**32,
    "sigmasign": 1.010,
    "sigmaverify": 1.042,
    "sigmakrsec": 1.042,
    "lensalt": 112 // 8,
    "lenkgseed": 128 // 8,
    "lenhpub": 128 // 8,
    "low00": 5,
    "high00": 9,
    "low01": 8,
    "high01": 11,
    "high11": 13,
    "highs0": 12,
    "lows1": 5,
    "highs1": 9,
    "beta0": 1 / 250,
    "T0": Hawk256_T0,
    "T1": Hawk256_T1,
}

HAWK_512_PARAMS = {
    "lenpriv": 184,
    "lenpub": 1024,
    "lensig": 555,
    "n": 512,
    "qs": 2**64,
    "sigmasign": 1.278,
    "sigmaverify": 1.425,
    "sigmakrsec": 1.425,
    "lensalt": 192 // 8,
    "lenkgseed": 192 // 8,
    "lenhpub": 256 // 8,
    "low00": 5,
    "high00": 9,
    "low01": 9,
    "high01": 12,
    "high11": 15,
    "highs0": 13,
    "lows1": 5,
    "highs1": 9,
    "beta0": 1 / 1000,
    "T0": Hawk512_T0,
    "T1": Hawk512_T1,
}

HAWK_1024_PARAMS = {
    "lenpriv": 360,
    "lenpub": 2440,
    "lensig": 1221,
    "n": 1024,
    "qs": 2**64,
    "sigmasign": 1.299,
    "sigmaverify": 1.571,
    "sigmakrsec": 1.974,
    "lensalt": 320 // 8,
    "lenkgseed": 320 // 8,
    "lenhpub": 512 // 8,
    "low00": 6,
    "high00": 10,
    "low01": 10,
    "high01": 14,
    "high11": 17,
    "highs0": 14,
    "lows1": 6,
    "highs1": 10,
    "beta0": 1 / 3000,
    "T0": Hawk1024_T0,
    "T1": Hawk1024_T1,
}


def PARAMS(logn, param):
    match logn:
        case 8:
            return HAWK_256_PARAMS[param]
        case 9:
            return HAWK_512_PARAMS[param]
        case 10:
            return HAWK_1024_PARAMS[param]
        case _:
            raise Exception(f"{logn} logn is not supported")
