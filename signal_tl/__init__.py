from signal_tl._cext import (Always, And, Const, Eventually, Not, Or,
                             Predicate, Until)
from signal_tl._cext.semantics import compute_robustness
from signal_tl._cext.signal import Sample, Signal, Trace, synchronize
from signal_tl._version import __version__

F = Eventually
G = Always
U = Until

TOP = Const(True)
BOT = Const(False)
