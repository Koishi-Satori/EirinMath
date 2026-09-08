Vector Library
==============

The vector library provides the fixed-size vector template ``tvec<N, T>``
with partial specializations for ``N == 2``, ``N == 3`` and ``N == 4``.
Components are plain members of type ``T`` and all operations are
``constexpr``, so vectors work in constant expressions and with any scalar
type the component operators support: standard integers, floating point
types and the fixed point types of this library.

Everything below is provided by the ``vec.hpp`` header, which is also
included by the umbrella header ``eirin.hpp``.

Type Aliases
------------

``vec2``, ``vec3`` and ``vec4`` are alias templates over ``tvec<N, T>``::

    vec2<int> a;      // tvec<2, int>
    vec3<float> b;    // tvec<3, float>
    vec4<fixed32> c;  // tvec<4, fixed32>

Concrete aliases follow a naming scheme of ``vec`` + size + component kind:

- native-width integers: ``vec2i``/``vec2u``, ``vec3i``/``vec3u``,
  ``vec4i``/``vec4u`` (``int`` / ``unsigned int``);
- sized integers: ``vec2i8``...``vec4u64`` with explicit bit widths
  (``int8_t`` ... ``uint64_t``), and the 128-bit variants
  (``vec2i128``...) when the extended integer support is available;
- floating point: ``vec2f``/``vec2d``... (``float``/``double``), plus the
  optional ``std::float16_t``/``float32_t``/``float64_t`` aliases when the
  corresponding extended floating point types are available;
- fixed point: generic alias templates ``vec2fixed<T, I, f, r>``...
  and concrete aliases ``vec2fixed32``/``vec2fixed64``...

Construction and Conversion
---------------------------

- default construction (value-initialize with ``{}`` to zero the
  components);
- a single scalar fills every component (``explicit``);
- one scalar per component;
- GLSL-style composition from smaller vectors, e.g. a ``vec3`` can be built
  from a ``vec2`` plus a scalar or from the leading components of a
  ``vec4``;
- conversion from another vector of the same size: each component is
  converted with ``static_cast``.

.. code-block:: c++

    using namespace eirin;

    vec2i zero{};                // (0, 0)
    vec3i fill(7);               // (7, 7, 7)
    vec4i v(1, 2, 3, 4);         // per component
    vec3d w(vec2d(1.5, 2.5), 3); // vec2 + scalar
    vec3i u = v.xyz();           // conversion from a swizzle result

Element Access
--------------

Components are public members named ``x``, ``y``, ``z`` and ``w`` and can be
indexed with ``operator[]``:

.. code-block:: c++

    vec4i v(1, 2, 3, 4);
    v.x = 10;            // member access, writable
    int a = v[2];        // index access, a == 3
    v[1] = 20;           // v == (10, 20, 3, 4)

``tvec<N, T>::size()`` is a compile-time constant returning ``N``. Indices
are not range checked; use values in ``[0, N)``.

Arithmetic and Logical Operators
--------------------------------

Binary operators are element-wise. A vector can be combined with another
vector of the same size or with a scalar on either side; when component
types differ, the right-hand component is converted to the left-hand
component type.

- arithmetic: ``+ - * / %`` and their compound forms ``+= -= *= /= %=``;
- bitwise: ``& | ^ << >>`` (available when ``T`` supports the operator) and
  their compound forms, plus unary complement ``~``;
- unary: ``+``, ``-`` and pre/post ``++``/``--``;
- comparisons: ``==`` compares all components exactly; ``!=`` is available
  through the rewritten comparison rules;
- ``&&`` and ``||`` return a ``bool`` vector component-wise.

The base class also provides:

- ``dot(rhs)``: sum of the component-wise products, returning ``value_type``;
- ``cross(rhs)``: for 2-component vectors a scalar (``x * y' - y * x'``),
  for 3-component vectors the usual vector cross product;
- ``nearly_eq(rhs)``: tolerance-based comparison, component-wise. Fixed
  point components use the epsilon configured through
  ``detail::nearly_compare_epsilon``; built-in floating point components use
  a per-type epsilon.

.. code-block:: c++

    using namespace eirin;

    vec4i a(1, 2, 3, 4);
    vec4i b(5, 6, 7, 8);
    auto s = a + b;            // (6, 8, 10, 12), element-wise
    auto t = b * 2;            // scalar on the right
    auto u = 2 * b;            // scalar on the left
    auto d = a.dot(b);         // 1*5 + 2*6 + 3*7 + 4*8 == 70
    vec3f c(1.0f, 0.0f, 0.0f);
    vec3f d3(0.0f, 1.0f, 0.0f);
    auto n = c.cross(d3);      // (0, 0, 1)

Swizzle Operations
------------------

Swizzle selectors use function-call syntax and the component names
``x``/``y``/``z``/``w``. The mask length is 2, 3 or 4 and determines the
result::

    vec4i v(1, 2, 3, 4);
    auto r2 = v.xy();          // selects (1, 2)
    auto r3 = v.wzy();         // reordered (4, 3, 2)
    auto r4 = v.xyxy();        // repeated components are allowed

Reading is available on every selector, including repeated and reordered
ones, and the result participates in expressions as a value. Writes follow
the GLSL rules:

- a selector without repeated components is writable and writes back to the
  original vector: ``v.yx() = ...`` stores the lanes in ``y, x`` order;
- a selector with repeated components cannot be used as an l-value, neither
  as a whole mask, through ``operator[]`` nor through single-component
  access;
- ``const`` vectors and selectors over them are read-only;
- assigning a scalar to a writable selector broadcasts it to every selected
  lane (an extension; GLSL requires an explicit vector constructor);
- overlapping assignments snapshot the right-hand side first, so
  ``v.xy() = v.yx()`` swaps the two components.

.. code-block:: c++

    using namespace eirin;

    vec4i v(1, 2, 3, 4);
    vec2i s = v.yx();          // materialized copy (2, 1)
    v.yx() = vec2i(9, 8);      // v == (8, 9, 3, 4)
    v.xy() = 7;                // scalar broadcast, v == (7, 7, 3, 4)

    vec2i ov(1, 2);
    ov.xy() = ov.yx();         // snapshot semantics: ov == (2, 1)

Compound assignments and ``++``/``--`` are supported on writable selectors
with the same snapshot semantics.

Swizzle proxies
^^^^^^^^^^^^^^^

Selectors return a light-weight ``swizzle_proxy`` view that keeps a pointer
to the original vector. The view converts implicitly to a ``tvec`` of the
mask length (or can be materialized with ``operator()``), so it is usually
invisible.

.. warning::
    A proxy refers to the vector it was created from. Do not store a proxy
    longer than the original vector lives; materialize it with an explicit
    conversion (``tvec`` copy or ``operator()``) when a stable value is
    needed.

Chained swizzles (e.g. ``v.xy().yx()``) compose selectors and keep the
component indices of the original vector. By default a chained selector is
read-only: chained reads work, while using a chained result as an l-value is
rejected at compile time, matching the conservative reading of GLSL. The
optional ``EIRIN_VEC_ENABLE_SWIZZLE_CHAIN_AS_LVALUE`` mode makes every chain
hop writable and performs the write as a flattened mask.

Configuration
-------------

The swizzle feature can be configured with the following macros. Define
them before including any EIRIN header, or set them directly in
``ext/config.hpp``:

- ``EIRIN_VEC_SWIZZLE_ENABLE`` (default ``EIRIN_ENABLE``): master switch of
  the swizzle API;
- ``EIRIN_VEC_SWIZZLE_CHAIN`` (default ``EIRIN_ENABLE``): allow selectors on
  proxy results (the legacy spelling ``EIRIN_VEC_DISABLE_SWIZZLE_CHAIN``
  disables chaining);
- ``EIRIN_VEC_SWIZZLE_CHAIN_AS_LVALUE`` (default ``EIRIN_DISABLE``): make
  chained selectors writable l-values (the legacy spelling
  ``EIRIN_VEC_ENABLE_SWIZZLE_CHAIN_AS_LVALUE`` enables it);
- ``EIRIN_VEC_SWIZZLE_FORCE_INLINE`` (default ``EIRIN_DISABLE``): force
  inlining of the generated selector functions.

Disabling the feature or chaining removes the corresponding member
functions entirely, so misuse fails to compile instead of being silently
ignored.
