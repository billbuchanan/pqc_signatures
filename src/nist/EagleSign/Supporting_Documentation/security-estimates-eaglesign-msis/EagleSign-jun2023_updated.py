from MSIS_security import MSIS_summarize_attacks, MSISParameterSet


class UniformEagleSignParameterSet(object):
    def __init__(self, n, k, l, etay2, etay1, t, etag, etad, tau, q):
        self.n = n
        self.k = k
        self.l = l
        self.etay1 = etay1
        self.etay2 = etay2
        self.etag = etag
        self.etad = etad
        self.tau = tau
        self.t = t
        self.q = q

        delta = l*(t+tau)*etag
        deltaprime = l*(t+tau)*etad + etay2
        self.beta = max(2*deltaprime, 2*delta)


UnifEagleSignMedium = UniformEagleSignParameterSet(
    512, 2, 1, 64, 1, 140, 6, 6, 18, 12289)
UnifEagleSignRecommended_I = UniformEagleSignParameterSet(
    1024, 1, 1, 64, 1, 140, 1, 1, 38, 12289)
UnifEagleSignRecommended_II = UniformEagleSignParameterSet(
    512, 2, 2, 32, 1, 90, 2, 1, 18, 12289)
UnifEagleSignVeryHigh_I = UniformEagleSignParameterSet(
    1024, 1, 2, 32, 1, 86, 1, 1, 18, 12289)

all_params_unif = [
    ("Uniform EagleSign Medium", UnifEagleSignMedium),
    ("Uniform EagleSign Recommended I", UnifEagleSignRecommended_I),
    ("Uniform EagleSign Recommended II", UnifEagleSignRecommended_II),
    ("Uniform EagleSign Very High I", UnifEagleSignVeryHigh_I),
]


def EagleSign_to_MSIS(dps):
    return MSISParameterSet(dps.n, dps.k + dps.l + dps.l, dps.k, dps.beta, dps.q, norm="linf")


if __name__ == "__main__":
    for (scheme, param) in all_params_unif:
        print("\n"+scheme)
        print(param.__dict__)

        print("")
        print("=== STRONG UF")

        MSIS_summarize_attacks(EagleSign_to_MSIS(param))

