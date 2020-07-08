import signal_tl as stl

# Signal(sample_values, sample_times)
xs = stl.Signal([0, 2, 1, -2, -1], [0, 2.5, 4.5, 6.5, 9])
ys = stl.Signal([0, -2, 2, 1, -1.5], [0, 2, 6, 8.5, 11])

# Synchronizing signals
xs_, ys_ = stl.synchronize(xs, ys)

print("x: {}".format(list(xs)))
print("y: {}".format(list(ys)))
print("\nSyncronizing the times")
print("xs_.time: {}".format(list(map(lambda s: s.time, xs_))))
print("ys_.time: {}".format(list(map(lambda s: s.time, ys_))))

print("\nSynchronized signals (With piecewise linear interpolation)")
print("xs_: {}".format(list(xs_)))
print("ys_: {}".format(list(ys_)))
