# About

Trying out things with: wg21.link/p0201

Found an interesting way (I think) to implement it:
using std::function to do type erasure.
It's smaller than refernce implementation and gives me small buffer implementaion for free.
It means that I'd practically never'd go allocating Copier/Deleter separatly on the heap - which is nice.

# Complains about the proposal

Even though I think the proposal is fantastic, I have a few objections:

* Exception on slicing

Slicing is a programming error - we don't typically do exceptions for those.
If contracts make it - that should be a contract violation.
If not - it can be undefined behaviour + assert.

The propper implementation of this check requires RTTI enabled, which is not the case for many
and violates the zero overhead principle.

* Construction from both T* and T& converting.

I understand the intention - both of those things should be convertible to polymorphic value.
However, I believe this is very error prone and confusing.
I'd suggest to have an std::in_place https://en.cppreference.com/w/cpp/utility/in_place for inplace construction.
I would also not do the conversion from raw pointer.

* I'd suggest to change constructor from T* + (optional copier and deleter) to std::unique_ptr<T, D> + Copier.

This is consistent with the guideline of not having owning raw pointers.

Current interface can lead to memory leaks, consider:

```
std::vector<std::polymorfic_value<A>> v;
/*
  fill the v
*/
v.emplace_back(new T{/*args*/});  // This leaks if the exception is thrown.
```

* Under TODO:

The thing that I don't like about this type (it's OK, but we could be more efficient) is the necessaty to do runtime dispatch
Through separate means to the provided interface. Sure, if we are just adapting the existing interface - we cannot do much about it
(at least I do not know how).
However - I think it's possible to provide a crtp base to the interface specifically designed for polymorphic_value
(maybe with additional requirements like - it has to be the first base)
and then do runtime dispatch throught the same pointer as the user. This way we avoid having two runtime dispatch mechanisms
and get closer to zero overhead over hand written implemented type_erasure.
I don't think you can get zero overhead without meta-classes, but it'd be really close.


# Other things

* In support of polymorphic value being SemiRegular vs Regular.

Originally when I saw the proposal I thought that not having equality is wrong - since we control the actual type stored inside,
there are a few quite easy things one could do to get cheap and efficient coded rrti.

However, I did not consider that one of the important use-cases for polymorphic value is to hide legacy 'clone' methods.
It's entirely possible that at the point of the user trying to incapsulate that, she'd not know the most derived type.
Since the user would not know it, we could not generate the efficient rtti.
Relying on compiler provided RTTI is unaccaptable.

I don't know how clear is that but basically if I'm not  it's either Copier or Regularity,
and I tend to think that providing custom Copier is much more important.