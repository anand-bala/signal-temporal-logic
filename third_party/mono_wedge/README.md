# STL Monotonic Wedge

## Lemire-Fenn algorithm for rolling min/max

Computing the minimum and maximum values in a "rolling" range --- such as the last second of an audio signal --- is a common problem in DSP, particularly in "peak finding" problems which often arise in transient detection and compressor design.  They can also be used to observe trends in gradually-changing values.

The brilliantly simple Lemire algorithm allows for computing the rolling minimum and maximum values in amortized constant time per sample, regardless of range.  The algorithm implemented here incorporates enhancements proposed by Ethan Fenn to lower the worst-case complexity per sample from **O(N)** to **O(log2(N))**.

The algorithm works by maintaining a "monotonic wedge", comprising those values which compare greater (or less) than all values following them and ending with the latest value.  Each time a new value is added to the wedge, any values which are not greater (or less) than the latest value are removed from the end of the wedge, after which the new value is appended.

_(Think of it like gazing at a mountain range from a great distance.  If a new mountain were to appear, it would obscure the foothills behind it --- but not the higher peaks beyond.)_

The "front" (oldest) value of the wedge is always the greatest (or least) of all.  Typically values beyond a certain age are removed (via `pop_front` or similar) in order to evaluate the largest (or smallest) value in a finite range.  Even if this is not done, the algorithm will update in amortized linear time---only the worst-case time per sample will increase!

Read the original Lemire paper [here](https://arxiv.org/abs/cs/0610046).


### Using the Code

Only `mono_wedge.h` is necessary to utilize this algorithm in a C++ application.

The implementation here conforms to the C++/STL programming style and may be used with `std::deque` and `std::vector` templates in addition to others satisfying the requirements (below).

#### Adding Samples

```python
min_wedge_update (wedge, value)
max_wedge_update (wedge, value)
mono_wedge_update(wedge, value, comp)
```

Call these functions with an initally empty container in order to maintain a monotonic wedge of minimum or maximum values.  The first (oldest) element in the container will always be the minimum (or maximum) with subsequent values gradually increasing (or decreasing) until the latest.

If using `mono_wedge_update`, supplying a "less" function as `comp` produces a monotonically-increasing "min-wedge", while supplying a "greater" function results in a monotonically-decreasing "max-wedge".

**Complexity:**  N updates to an initially empty container will take **O(N)** time.  The worst case for a single update is **O(log2(N))** time.  For a "rolling" window where samples only remain in the container for a maximum of **W** steps, the worst-case for a single update is **O(log2(W))**.

The container must fulfill the requirements below.  `std::vector` and `std::deque` work well.

#### Retrieving the Minimum/Maximum

No function is provided; simply query the front item of the container.

#### "Rolling" Minimum/Maximum

No function is provided; simply pop the back item(s) from the container whenever your chosen limits on age or wedge size are exceeded.

**Complexity**:  **O(N+log2(W))** for **N** updates if the container holds at most **W** elements.  (This is where the Lemire-Fenn algorithm has an advantage over the original Lemire algorithm.)

#### Lemire-Fenn Search Subroutine

_These are subroutines of the `wedge_update` functions and do not need to be used directly._

```python
min_wedge_search (begin, end, value)
max_wedge_search (begin, end, value)
mono_wedge_search(begin, end, value, comp)
```

These functions behave similar to `std::lower_bound`, returning the first element which does not satisfy `comp(value, element)`.

**Complexity:**  **O(log2(N))** time in the worst case, where N is the number of elements in the wedge.  Uses a combination of linear and binary search in order to facilitate amortized constant-time execution of `wedge_update` routines.

The iterators must fulfill the requirements below.  `std::vector` and `std::deque` work well.


### Requirements

`wedge_search` functions may be used with any range of random access iterators, including those provided by `std::vector` and `std::deque`.

`wedge_update` functions require that the wedge class produces random access iterators from its `begin()` and `end()` methods, and supports appending elements via a `push_back` method.  Again, `std::vector` and `std::deque` satisfy these requirements.


## Future Work

DSP applications may prefer to use a fixed-size ringbuffer as the underlying container for a wedge of bounded size.  The documentation should be updated with a recommendation for an STL-compliant ringbuffer implementation.

In addition to the `<algorithm>`-inspired functions here, it may be convenient to implement a wedge container type.  This could behave similar to `std::queue`, which is internally implemented using `std::deque`.

I may or may not pursue these enhancements myself, and welcome pull requests.


## License

This code is made available under the MIT license, and the underlying algorithm is unencumbered by patents at the time of writing.

See the LICENSE file for full copy.


## Credits & History

The original algorithm was designed by Daniel Lemire:  [See here](https://github.com/lemire/runningmaxmin) for his implementation [or here](https://arxiv.org/abs/cs/0610046) for the paper.

The enhanced algorithm was proposed on the music-dsp mailing list by Ethan Fenn and implemented here by Evan Balster.  Read the mailing list message where the Lemire-Fenn algorithm was proposed [here](https://lists.columbia.edu/pipermail/music-dsp/2016-July/000908.html) and a discussion of its first implementation [here](https://lists.columbia.edu/pipermail/music-dsp/2016-September/001083.html).

Ethan Fenn's original proposal:

```
    Ethan Fenn
    Wed Jul 20 10:27:27 EDT 2016

Of course, for processing n samples, something that is O(n) is going to
eventually beat something that's O(n*log(w)), for big enough w.

FWIW if it's important to have O(log(w)) worst case per sample, I think you
can adapt the method of the paper to achieve this while keeping the O(1)
average.

Instead of a linear search back through the maxima, do a linear search for
log2(w) samples and, failing that, switch to binary search. This is
O(log(w)) worst case, but I think it would keep the property of O(1)
comparisons per sample. I believe the upper bound would now be 4
comparisons per sample rather than 3. I'd have to think more about it to be
sure.

-Ethan
```

