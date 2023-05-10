#ifndef math_tools
#define math_tools

#include <algorithm>
#include <array>
#include "glm/glm.hpp"


namespace undicht {

    namespace tools {

        template<typename T>
        void swap(T& t0, T& t1) {
            T tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        template<typename number>
        bool overlappingRanges(number range_start_1, number range_end_1, number range_start_2, number range_end_2) {
            /** "touching" ranges dont overlap */

            // making sure the start of the range is before the end
            if(range_start_1 > range_end_1) swap(range_start_1, range_end_1);
            if(range_start_2 > range_end_2) swap(range_start_2, range_end_2);

            // cases in which the ranges do not overlap (is it really that easy ?)
            if((range_end_1 <= range_start_2) || (range_end_2 <= range_start_1)) 
                return false;

            return true;
        }


        template<typename number>
        bool overlappingRanges(number c0_0, number c0_1, number c1_0, number c1_1, number& mid0, number& mid1) {
            /// calculates whether the ranges between c0_0 to c0_1 and c1_0 to c1_1 overlap
            /// if they do, the values in the middle get stored in mid1 and mid2
            /// @return false, if they dont overlap
            
            // make sure c0_0 is smaller than c0_1
            if(c0_0 > c0_1) swap(c0_0, c0_1);
            if(c1_0 > c1_1) swap(c1_0, c1_1);

            // are the ranges overlapping? (touching doesnt count as overlapping)
            if(c0_1 <= c1_0) return false; // is range c0 totally left of c1? 
            if(c0_0 >= c1_1) return false; // is range c0 totally right of c1? 

            // how are the ranges overlapping ?
            if(c0_0 >= c1_0) mid0 = c0_0; else mid0 = c1_0;
            if(c0_1 >= c1_1) mid1 = c1_1; else mid1 = c0_1;

            return true;
        }

        template<typename number>
        std::array<number,2> getMidValues(number n0, number n1, number n2, number n3) {
            /** @return not the biggest and not the smallest number */

            // there may be a faster way
            std::array<number, 4> sorted = {n0, n1, n2, n3};
            std::sort(sorted.begin(), sorted.end());

            return {sorted[1], sorted[2]};
        }


    } // tools
 
} // undicht


#endif // math_tools
