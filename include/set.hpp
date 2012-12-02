#include <vector>
#include <algorithm>
#include <ostream>
#include <exception>
#include <iterator>

#pragma once


namespace cordite {

    template <typename T, class Allocator = ::std::allocator<T>>
class set {
    typedef set<T,Allocator> _me;
    typedef ::std::vector<T,Allocator> _vme;
    typedef _vme _dme;
private:
    _dme _data;
public:
    set(){}
    ~set(){}
    set(const _me & other)
    {
        _data = other._data;
    }
    set(const _dme & data)
    {
        _data = data;
        sort();
        uniqueCheck();
    }
    set(_dme && data)
    :_data(data)
    {
        sort();
        uniqueCheck();
    }
    set(_me && other)
    :_data(::std::move(other._data))
    {
    }
    typename _dme::iterator begin() {
        return _data.begin();
    }
    typename _dme::iterator end() {
        return _data.end();
    }
    typename _dme::const_iterator begin() const{
        return _data.cbegin();
    }
    typename _dme::const_iterator end() const{
        return _data.cend();
    }
    _me unionWith(const _me & other) const
    {
        _me r;
        ::std::set_union
        (_data.begin(),_data.end(),
         other._data.begin(),other._data.end(),
         ::std::inserter(r._data,r._data.end()));
        return r;
    }
    _me intersectWith(const _me & other) const
    {
        _me r;
        ::std::set_intersection(
            _data.begin(),_data.end(),
            other._data.begin(),other._data.end(),
            ::std::inserter(r._data,r._data.end()));
        return r;
    }
    _me differenceWith(const _me & other) const
    {
        _me r;
        ::std::set_difference
        (_data.begin(),_data.end(),
         other._data.begin(),other._data.end(),
         ::std::inserter(r._data,r._data.end()));
        return r;
    }
    _me symmetricDifferenceWith(const _me & other) const
    {
        _me r;
        ::std::set_symmetric_difference
        (_data.begin(),_data.end(),
         other._data.begin(),other._data.end(),
         ::std::inserter(r._data,r._data.end()));
        return r;
    }
    
    _me augmentWith(const T & val) const
    {
        _me r(_data);
        r.insert(val);
        return r;
    }
    
    void insert(const T & val)
    {
        _data.push_back(val);
        sort();
        uniqueCheck();
    }
    void insert(const _me & other)
    {
        _data.reserve(_data.size()+other._data.size());
        for_each(other.begin(),other.end(),[&](const T & s){
            _data.push_back(s);
        });
        sort();
        uniqueCheck();
    }
    template<class HigherAllocator = ::std::allocator<set<T,Allocator>>>
    set<set<T,Allocator>,HigherAllocator> powerset() {
        //return foreach_powerset();
        return binary_powerset();
    }
    friend bool operator<(const _me & first, const _me & other) {
        if(first._data.size() < other._data.size()) return true;
        if(first._data.size() > other._data.size()) return false;
        auto it = first.begin();
        auto it2 = other.begin();
        for(;it != first.end() && it2 != other.end();
            it++, it2++)
        {
            if(*it < *it2) return true;
            if(*it > *it2) return false;
        }
        return false;
    }
    friend bool operator==(const _me & first, const _me & other) {
        bool ret = false;
        if(first._data.size() != other._data.size()) return false;
        for_each(first._data.begin(),first._data.end(),[&] (const T & val){
            for_each(other._data.begin(),other._data.end(),[&] (const T & val2){
                if(val != val2)
                {
                    ret = true;
                    return;
                }
            });
            if(ret) return;
        });
        return !ret;
    }
    _me operator=(_me && other) {
        if(&other != this)
        {
            _data.clear();
            _data = std::move(other._data);
        }
        return *this;
    }
    _me & operator=(const _me & other)
    {
        _data = other._data;
        return *this;
    }

    friend ::std::ostream &operator<<(::std::ostream &out, const set & s)
    {
        out << '[';
        bool hasone = false;
        for_each(s._data.begin(),s._data.end(),[&](const T & t) {
            if(hasone) out << ", ";
            out << t;
            hasone = true;
        });
        out << ']';
        return out;
    }
    void deferinsert(const T & val)
    {
        _data.push_back(val);
    }
    void deferinsert(T && val)
    {
        _data.push_back(val);
    }
    void cleanup()
    {
        sort();
        uniqueCheck();
    }
    void uniqueCheck()
    {
        typename _dme::iterator it = ::std::unique(_data.begin(),_data.end());
        _data.resize(int(it - _data.begin()));
    }
    void sort()
    {
        ::std::sort(_data.begin(),_data.end());
    }
    void reserve(size_t size)
    {
        _data.reserve(size);
    }
    class power_iterator : ::std::iterator<::std::output_iterator_tag,T>
    {
        friend _me;
    public:
        power_iterator(const power_iterator & other):
        //representation(other.representation),
        ret_data(other.ret_data),
        origin_data(other.origin_data),
        origin_size(other.origin_size),
        //origin_representation(other.origin_representation),
        teir(other.teir),
        stack(other.stack)
        {
             ret_data.reserve(origin_size);
        }
        power_iterator & operator++()
        {
            ret_data.resize(0);
            /*unsigned char window = 0;
            //Calculate the new representation.
            unsigned char bit_position;
            for(bit_position = origin_size;
                bit_position > 0;
                --bit_position)
            {
                //Find a zero
                if(!((size_t)1 << bit_position & representation))
                {
                    window |= ZERO_FOUND;
                    break;
                }
            }
            while (--bit_position > 0 && (window & ZERO_FOUND) && teir < origin_size)
            {
                if(((size_t)1 << bit_position & representation))
                {
                    representation ^= (size_t)1 << bit_position;//set current bit to 0
                    representation ^= (size_t)1 << (bit_position + 1);//set the previous (known) 0 bit to 1
                    window |= ONE_MOVED;
                    break;
                }
                else
                {
                    while(--bit_position > 0)
                    {
                        if(((size_t)1 << bit_position) & representation)
                        {
                            
                        }
                    }
                }
            }
            
            if((window & ZERO_FOUND) && (window & ~(size_t)ONE_MOVED))
            {
                //If a zero was found, but we never moved a 1, then we must have
                //hit the end of the teir we are within.
                //We need to calculate the next teir.
                //TODO
                ++teir;
            }
            if(!window)
            {
                //We never found anything, we must be at the end.
            }*/
            if(!incriment_powerset(origin_data,stack, teir))
            {
                if(teir < origin_size)
                {
                    stack.resize(++teir);
                    ::std::fill(stack.begin(),stack.end(),-1);
                    incriment_powerset(origin_data,stack, teir);
                }
                else
                {
                    teir=origin_size+1;
                }
            }
            //Generate the new data.
            build_set();
            return *this;
        }
        power_iterator operator++(int)
        {
            power_iterator tmp(*this);
            operator++();
            return tmp;
        }
        bool operator==(const power_iterator& other) {
            return &origin_data == &other.origin_data
            && teir == other.teir
            && ::std::equal(stack.begin(),stack.end(),other.stack.begin());
        }
        bool operator!=(const power_iterator& other) {
            return !operator==(other);
        }
        const _dme & operator*() {
            return ret_data;
        }
    private:
        power_iterator(const _dme & data):
        origin_data(data),
        origin_size(::std::min(data.size(),sizeof(size_t)*8)),
        //representation(rep_from_size(data.size())),
        //origin_representation(rep_from_size(data.size())),
        teir(0)
        {
            ret_data.reserve(origin_size);
        }
        //constructor for end_iterator
        power_iterator(const _dme & data, bool):
        origin_data(data),
        origin_size(::std::min(data.size(),sizeof(size_t)*8))
        {
            teir = origin_size+1;
            stack.reserve(teir);
            for(int i = 0; i < teir; ++i)
            {
                stack.push_back(i);
            }
            build_set();
        }
        _dme ret_data;
        const _dme & origin_data;
        const size_t origin_size;
        //const size_t origin_representation;
        //size_t representation;
        ::std::vector<size_t> stack;
        unsigned char teir;
        
        static const size_t rep_from_size(size_t size)
        {
            size=::std::min(size,sizeof(size_t)*8);
            return ~((size_t)~0 >> size);
        }
        void build_set()
        {
            /*unsigned char bit_position;
            size_t sub_representation = 0;
            size_t hits = 0;
            for(bit_position = 0; bit_position < origin_size; ++bit_position)
            {
                sub_representation = (size_t)1 << bit_position;
                if(sub_representation & representation)
                {
                    ++hits;
                    ret_data.push_back(origin_data[bit_position]);
                }
            }*/
        
            for_each(stack.begin(),stack.end(),[this] (const size_t index){
               ret_data.push_back(origin_data[index]);
            });
        }
        enum statuses
        {
            ZERO_FOUND = 1,
            ONE_MOVED = 2,
        };
        /*void reseat_block(unsigned char begin, unsigned char end, unsigned char amount)
        {
            const size_t mask =
            (
                ~(size_t)0 << (sizeof(size_t)*8)-end
            ) | (
                ~(size_t)0 >> (sizeof(size_t)*8)-begin
            );
            
            representation = (representation & mask) | ((~(size_t)0 << (begin + amount)) ^ ~(size_t)0 << begin);
        }*/
        
    };
    
    power_iterator power_begin()
    {
        return power_iterator(_data);
    }
    power_iterator power_end()
    {
        return power_iterator(_data,true);
    }
private:
    
    
    template<class HigherAllocator = ::std::allocator<set<T,Allocator>>>
    set<set<T,Allocator>,HigherAllocator> foreach_powerset()
    {
        typedef set<set<T,Allocator>,HigherAllocator> _ofme;
        _ofme r({_me()});
        for_each(_data.begin(),_data.end(),[&](const T & val){
            _ofme rs;
            for_each(r.begin(),r.end(),[&](const _me & s){
                rs.deferinsert(s.augmentWith(val));
            });
            r.insert(rs);
        });
        r.sort();
        return r;
    }
    template<class HigherAllocator = ::std::allocator<set<T,Allocator>>>
    set<set<T,Allocator>,HigherAllocator> binary_powerset()
    {
        typedef set<set<T,Allocator>,HigherAllocator> _ofme;
        size_t total = 1 << _data.size();
        _ofme r;
        r.reserve(total);
        r.deferinsert(_me());
        ::std::vector<size_t> stack(_data.size()+1);
        for(size_t i = 0; i < _data.size();++i)
        {
            ::std::fill(stack.begin(),stack.end(),-1);
            while(incriment_powerset(_data,stack, i))
            {
                _me subr;
                subr._data.reserve(i+1);
                for(size_t j = 0; j < i; ++j)
                {
                    subr.deferinsert(_data[stack[j]]);
                }
                r.deferinsert(::std::move(subr));
            }
        }
        return r;
        
    }
    static bool incriment_powerset(const _dme & _data,::std::vector<size_t> & stack, size_t stack_size, size_t position = 0)
    {
        if(!stack_size) return false;
        if(stack_size <= position) return false;
        if(stack[position] == -1)
        {
            stack[position] = 0;
            for(size_t i = position+1; i < stack_size; ++i)
            {
                stack[i] = stack[i-1] + 1;
            }
            return true;
        }
        if(stack[position] == _data.size() - (stack_size-position)) return false;
        if(position == stack_size - 1)
        {
            stack[position] += 1;
            return true;
        }
        if(incriment_powerset(_data, stack, stack_size,position+1))
        {
            return true;
        }
        else
        {
            stack[position] += 1;
            for(size_t i = position+1; i < stack_size; ++i)
            {
                stack[i] = stack[i-1] + 1;
            }
            return true;
        }
    }
};

};