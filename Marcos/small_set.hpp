#ifndef _small_set_hpp_include_
#define _small_set_hpp_include_

#include "core.hpp"

template<typename T>
struct set {
	auto size() const {
		return items.size();
	}

	bool has(T const& v) const {
		for(auto const& item : items){
			if(v == item){ return true; }
		}
		return false;
	}

	set& add(T const& v){
		if(!has(v)){
			items.append(v);
		}
		return *this;
	}

	set& del(T const& v){
		x::usize i = 0;
		for(auto const& item : items){
			if(v == item){ break; }
			i += 1;
		}
		items.remove_unordered(i);
		return *this;
	}

	auto begin() {
		return items.begin();
	}

	auto end() {
		return items.end();
	}

	set intersect(set const& s) {
		auto inter = set<T>(items.get_allocator());
		for(auto const& v : items){
			if(s.has(v)){
				inter.add(v);
			}
		}
		return inter;
	}

	set join(set const& s){
		auto sum = set<T>(items.allocator());
		for(auto const& v : items){ sum.add(v); }
		for(auto const& v : s.items){ sum.add(v); }
		return sum;
	}

	set(x::allocator al)
		: items(al) {}

	set(x::allocator al, x::slice<T> s)
		: items(al)
	{
		for(auto const& e : s){
			add(e);
		}
	}

	x::slice<T> extract_data(){
		return items.extract_data();
	}

private:
	x::dynamic_array<T> items;
};


#endif /* Include guard */
