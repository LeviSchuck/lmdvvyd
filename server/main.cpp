#include "set.hpp"
#include <iostream>
#include <chrono>
#include <string>
#include <numeric>
#include <iomanip>
using namespace std;
int main()
{
    typedef std::chrono::high_resolution_clock Clock;
    
	cordite::set<int> a({
		1,5,40
	});
	cordite::set<int> b({
		1,2,3,4,5,6,7,8,9,0
	});
    cordite::set<int> h({1,2,3,4,5,6,7,8,9,0});
	cout << a << endl << b << endl;

	cout << a.unionWith(b) << endl;
	cout << a.differenceWith(b) << endl;
	cout << a.symmetricDifferenceWith(b) << endl;
	cout << a.intersectWith(b) << endl;
	//cout << "Powerset(b): ";
    const size_t max_powerset_size = 20;
    const size_t min_powerset_size = 1;
    const size_t max_attemps = 100;
    cout << "Powerset generation"<< endl;
    for(size_t size = min_powerset_size; size <= max_powerset_size; ++size)
    {
        std::vector<int> data(size);
        std::iota(data.begin(),data.end(),0);
        cordite::set<int> test(data);
        size_t total = 0;
        Clock::time_point t0 = Clock::now();
        for(size_t i = 0; i <= max_attemps; ++i)
        {
            auto u = test.powerset();
            total += 1;
        }
        //cout << c << endl;
        Clock::time_point t1 = Clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
        cout << "Took avg: "
        << std::setw(5) << std::setfill('_') << std::chrono::duration_cast<std::chrono::milliseconds>(ns).count() / total << "ms "
        << std::setw(8) << std::setfill('_') << std::chrono::duration_cast<std::chrono::microseconds>(ns).count() / total << "µs "
        << std::setw(12) << std::setfill('_') << ns.count() / total << "ns\t"
        << "for " << size << " items." << endl;
    }
    cout << "Iterations only"<< endl;
    for(size_t size = min_powerset_size; size <= max_powerset_size; ++size)
    {
        std::vector<int> data(size);
        std::iota(data.begin(),data.end(),0);
        cordite::set<int> test(data);
        size_t total = 0;
        Clock::time_point t0 = Clock::now();
        size_t totalish = 0;
        for(size_t i = 0; i <= max_attemps; ++i)
        {
            for_each(test.power_begin(),test.power_end(),[&](const ::std::vector<int> & s){
                //cout << cordite::set<int>(s) << endl;
                totalish += std::accumulate(s.begin(),s.end(),0);
            });
            ++total;
        }
        Clock::time_point t1 = Clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
        cout << "Took avg: "
        << std::setw(5) << std::setfill('_') << std::chrono::duration_cast<std::chrono::milliseconds>(ns).count() / total << "ms "
        << std::setw(8) << std::setfill('_') << std::chrono::duration_cast<std::chrono::microseconds>(ns).count() / total << "µs "
        << std::setw(12) << std::setfill('_') << ns.count() / total << "ns\t"
        << "for " << size << " items."
        << endl;

    }
    return 0;
}