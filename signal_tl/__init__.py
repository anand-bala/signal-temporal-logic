from signal_tl._cext import Or, And, Not, Const, Until, Always, Predicate, Eventually
from signal_tl._cext.signal import Trace, Sample, Signal
from signal_tl._cext.semantics import compute_robustness

F = Eventually
G = Always
U = Until
