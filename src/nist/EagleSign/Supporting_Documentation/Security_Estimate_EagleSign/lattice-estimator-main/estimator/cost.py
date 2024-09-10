# -*- coding: utf-8 -*-
from collections import UserDict

from sage.all import log, oo, round


# UserDict inherits from typing.MutableMapping
class Cost(UserDict):
    """
    Algorithms costs.
    """

    # An entry is "impermanent" if it grows when we run the algorithm again. For example, `δ`
    # would not scale with the number of operations but `rop` would. This check is strict such that
    # unknown entries raise an error. This is to enforce a decision on whether an entry should be
    # scaled.

    impermanents = {
        "rop": True,
        "repetitions": False,
        "tag": False,
        "problem": False,
    }

    @staticmethod
    def _update_without_overwrite(dst, src):
        keys_intersect = set(dst.keys()) & set(src.keys())
        attempts = [
          f"{k}: {dst[k]} with {src[k]}" for k in keys_intersect if dst[k] != src[k]
        ]
        if len(attempts) > 0:
            s = ", ".join(attempts)
            raise ValueError(f"Attempting to overwrite {s}")
        dst.update(src)

    @classmethod
    def register_impermanent(cls, data=None, **kwds):
        if data is not None:
            cls._update_without_overwrite(cls.impermanents, data)
        cls._update_without_overwrite(cls.impermanents, kwds)

    key_map = {
        "delta": "δ",
        "beta": "β",
        "beta_": "β'",
        "eta": "η",
        "eta_": "η'",
        "epsilon": "ε",
        "zeta": "ζ",
        "zeta_": "ζ'",
        "ell": "ℓ",
        "ell_": "ℓ'",
        "repetitions": "↻",
    }

    val_map = {"beta": "%8d", "beta_": "%8d", "d": "%8d", "delta": "%8.6f"}

    def str(self, keyword_width=0, newline=False, round_bound=2048, compact=False):
        """

        :param keyword_width:  keys are printed with this width
        :param newline:        insert a newline
        :param round_bound:    values beyond this bound are represented as powers of two
        :param compact:        do not add extra whitespace to align entries

        EXAMPLE::

            >>> from estimator.cost import Cost
            >>> s = Cost(delta=5, bar=2)
            >>> s
            δ: 5.000000, bar: 2

        """

        def value_str(k, v):
            kstr = self.key_map.get(k, k)
            kk = f"{kstr:>{keyword_width}}"
            try:
                if (1 / round_bound < abs(v) < round_bound) or (not v) or (k in self.val_map):
                    if abs(v % 1) < 1e-7:
                        vv = self.val_map.get(k, "%8d") % round(v)
                    else:
                        vv = self.val_map.get(k, "%8.3f") % v
                else:
                    vv = "%7s" % ("≈2^%.1f" % log(v, 2))
            except TypeError:  # strings and such
                vv = "%8s" % v
            if compact is True:
                kk = kk.strip()
                vv = vv.strip()
            return f"{kk}: {vv}"

        # we store the problem instance in a cost object for reference
        s = [value_str(k, v) for k, v in self.items() if k != "problem"]
        delimiter = "\n" if newline is True else ", "
        return delimiter.join(s)

    def reorder(self, *args):
        """
        Return a new ordered dict from the key:value pairs in dictionary but reordered such that the
        keys given to this function come first.

        :param args: keys which should come first (in order)

        EXAMPLE::

            >>> from estimator.cost import Cost
            >>> d = Cost(a=1,b=2,c=3); d
            a: 1, b: 2, c: 3

            >>> d.reorder("b","c","a")
            b: 2, c: 3, a: 1

        """
        reord = {k: self[k] for k in args if k in self.keys()}
        reord.update(self)
        return Cost(**reord)

    def filter(self, **keys):
        """
        Return new ordered dictionary from dictionary restricted to the keys.

        :param dictionary: input dictionary
        :param keys: keys which should be copied (ordered)
        """
        r = {k: self[k] for k in keys if k in self.keys()}
        return Cost(**r)

    def repeat(self, times, select=None):
        """
        Return a report with all costs multiplied by ``times``.

        :param times:  the number of times it should be run
        :param select: toggle which fields ought to be repeated and which should not
        :returns:      a new cost estimate

        EXAMPLE::

            >>> from estimator.cost import Cost
            >>> c0 = Cost(a=1, b=2)
            >>> c0.register_impermanent(a=True, b=False)
            >>> c0.repeat(1000)
            a: 1000, b: 2, ↻: 1000

        TESTS::

            >>> from estimator.cost import Cost
            >>> Cost(rop=1).repeat(1000).repeat(1000)
            rop: ≈2^19.9, ↻: ≈2^19.9

        """
        impermanents = dict(self.impermanents)

        if select is not None:
            impermanents.update(select)

        try:
            ret = {k: times * v if impermanents[k] else v for k, v in self.items()}
            ret["repetitions"] = times * ret.get("repetitions", 1)
            return Cost(**ret)
        except KeyError as error:
            raise NotImplementedError(
                f"You found a bug, this function does not know about about a key but should: {error}"
            )

    def __rmul__(self, times):
        return self.repeat(times)

    def combine(self, right, base=None):
        """Combine ``left`` and ``right``.

        :param left: cost dictionary
        :param right: cost dictionary
        :param base: add entries to ``base``

        EXAMPLE::

            >>> from estimator.cost import Cost
            >>> c0 = Cost(a=1)
            >>> c1 = Cost(b=2)
            >>> c2 = Cost(c=3)
            >>> c0.combine(c1)
            a: 1, b: 2
            >>> c0.combine(c1, base=c2)
            c: 3, a: 1, b: 2

        """
        base_dict = {} if base is None else base
        cost = {**base_dict, **self, **right}
        return Cost(**cost)

    def __bool__(self):
        return self.get("rop", oo) < oo

    def __add__(self, other):
        return self.combine(self, other)

    def __repr__(self):
        return self.str(compact=True)

    def __str__(self):
        return self.str(newline=True, keyword_width=12)

    def __lt__(self, other):
        try:
            return self["rop"] < other["rop"]
        except AttributeError:
            return self["rop"] < other

    def __le__(self, other):
        try:
            return self["rop"] <= other["rop"]
        except AttributeError:
            return self["rop"] <= other

    def sanity_check(self):
        """
        Perform basic checks.
        """
        if self.get("rop", 0) > 2**10000:
            self["rop"] = oo
        if self.get("beta", 0) > self.get("d", 0):
            raise RuntimeError(f"β = {self['beta']} > d = {self['d']}")
        if self.get("eta", 0) > self.get("d", 0):
            raise RuntimeError(f"η = {self['eta']} > d = {self['d']}")
        return self
