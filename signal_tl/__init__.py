from signal_tl._cext import Or, And, Not, Const, Until, Always, Predicate, Eventually
from signal_tl._cext.signal import Trace, Sample, Signal, synchronize
from signal_tl._cext.semantics import compute_robustness

F = Eventually
G = Always
U = Until

TOP = Const(True)
BOT = Const(False)
