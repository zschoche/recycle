/*
*
*	Author: Philipp Zschoche, https://zschoche.org
*
*/
#ifndef __RECYCLE_HPP__
#define __RECYCLE_HPP__

#include <memory>
#include <cstdlib>

namespace recycle {

namespace detail {

template <std::size_t S /* dustbin size*/, typename T, typename Allocator> struct ptr_buffer {

	typedef T *ptr_type;

	inline bool full() const { return m_count == S; }

	inline bool empty() const { return m_count == 0; }

	inline void push(T *ptr) { m_items[m_count++] = ptr; }

	inline T *pop() { return m_items[--m_count]; }

	Allocator allocator;

      private:
	ptr_type m_items[S];
	std::size_t m_count = 0;
};

template <typename T, typename Allocator, typename Container = ptr_buffer<128, T, Allocator> > Container *get_mem_waste() {
	thread_local std::unique_ptr<Container, void (*)(Container *)> t_ptrs(new Container(), [](Container * c) {
		while (!c->empty()) {
			c->allocator.deallocate(c->pop(), 1);
		}
		delete c;
	});
	return t_ptrs.get();
}

template <typename T, typename Allocator, typename... Args> T *make_ptr(Args &&... args) {
	auto *q = get_mem_waste<T, Allocator>();
	T *result;
	if (q->empty()) {
		result = q->allocator.allocate(1);
	} else {
		result = q->pop();
	}
	q->allocator.construct(result, std::forward<Args>(args)...);
	return result;
}

template <typename T, typename SmartPtr, typename Allocator, typename... Args> SmartPtr make_smart(Args &&... args) {
	return SmartPtr(detail::make_ptr<T, Allocator>(std::forward<Args>(args)...), [](T * ptr) {
		auto *q = detail::get_mem_waste<T, Allocator>();
		q->allocator.destroy(ptr);
		if (q->full()) {
			q->allocator.deallocate(ptr, 1);
		} else {
			q->push(ptr);
		}
	});
}

} /* namespace details */

template <typename T> using unique_ptr = std::unique_ptr<T, void (*)(T *)>;

template <typename T> using shared_ptr = std::shared_ptr<T>; // */

template <typename T, typename... Args> unique_ptr<T> make_unique(Args &&... args) {
	return detail::make_smart<T, unique_ptr<T>, std::allocator<T> >(std::forward<Args>(args)...);
}

template <typename T, typename... Args> shared_ptr<T> make_shared(Args &&... args) {
	return detail::make_smart<T, shared_ptr<T>, std::allocator<T> >(std::forward<Args>(args)...);
}

} /* namespace recycle */

#endif /* __RECYCLE_HPP__ */
