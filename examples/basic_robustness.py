import numpy as np

import signal_tl as stl

a = stl.Predicate("a") > 0
b = stl.Predicate("b") <= 0.5

phi = stl.Until(a, b)

t = np.linspace(0, 50, 201)
x = np.cos(t)
y = np.sin(t)

trace = {
    "a": stl.Signal(x, t),
    "b": stl.Signal(x, t),
}

rob = stl.compute_robustness(phi, trace)
print(rob.at(0))
