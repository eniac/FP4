#ifndef UTIL_CPP
#define UTIL_CPP

#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <regex>


static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

inline std::vector<size_t> find_all_positions(std::string s) {
    std::string sub = "valid(";

    std::vector<size_t> positions; // holds all the positions that sub occurs within str

    size_t pos = s.find(sub, 0);
    while(pos != std::string::npos)
    {
        positions.push_back(pos);
        pos = s.find(sub,pos+1);
    }
    return positions;
}

inline std::vector<size_t> find_all_end_positions(std::string s, std::vector<size_t> start_positions) {
    std::string sub = ")";

    std::vector<size_t> end_positions; // holds all the positions that sub occurs within str

    for(int i : start_positions) {
        end_positions.push_back(s.find_first_of(sub, i));
    }

    return end_positions;
}

inline std::string sanitizeName(std::string s) {
    std::vector<size_t> positions = find_all_positions(s);
    std::vector<size_t> end_positions = find_all_end_positions(s, positions);
    for (int i = end_positions.size() - 1; i >= 0; --i) {
        s.insert(end_positions[i], "isValid");
    }
    std::cout << "in sanitizeName" << std::endl;
    std::regex pattern("valid\\(");
    std::cout << "About to replace" << std::endl;
    s = std::regex_replace(s, pattern, "");    

    std::regex second_pattern("\\W+");
    std::string clean_string = std::regex_replace(s, second_pattern, "");
    
    return clean_string;
}


// template<typename T>
inline size_t RemoveDuplicatesKeepOrder(std::vector<std::string> vec)
{
    std::set<std::string> seen;

    auto newEnd = std::remove_if(vec.begin(), vec.end(), [&seen](const std::string& value)
    {
        if (seen.find(value) != std::end(seen))
            return true;

        seen.insert(value);
        return false;
    });

    vec.erase(newEnd, vec.end());

    return vec.size();
}

template<typename T>
inline bool isEqual(std::vector<T> const &v1, std::vector<T> const &v2)
{
    return (v1.size() == v2.size() &&
            std::equal(v1.begin(), v1.end(), v2.begin()));
}

inline std::vector<std::vector <std::string>> split_vector(std::vector<std::string> v, int bunch_size) {
    std::vector<std::vector <std::string> > rtn;
    for(size_t i = 0; i < v.size(); i += bunch_size) {
        auto last = std::min(v.size(), i + bunch_size);
        rtn.emplace_back(v.begin() + i, v.begin() + last);
    }
    return rtn;
}

//https://www.geeksforgeeks.org/how-to-convert-a-vector-to-set-in-c/
inline std::set<std::string> convertToSet(std::vector<std::string> v)
{
    // Declaring the set
    std::set<std::string> s;
  
    // Inserting the elements
    // of the vector in the set
    // using copy() method
    std::copy(
  
        // From point of the destination
        s.begin(),
  
        // From point of the destination
        s.end(),
  
        // Method of copying
        std::back_inserter(v));
  
    // Return the resultant Set
    return s;
}

// https://stackoverflow.com/a/29962411
template<typename Key, typename Value>
inline std::ostream& operator<<(std::ostream& os, const std::pair<const Key, Value>& p)
{
    os << p.first << " => " << p.second;
    return os;
}

template<typename Container>
inline void print_map(const Container& c) {
    for(typename Container::const_iterator it = c.begin();
            it != c.end(); ++it) {
        std::cout << *it << '\n';
    }
        
}

inline std::string int_to_hex(int numOnes)
{
    if (numOnes == 0) {
        return "0x0";
    }
    std::string binForm = "";
    for (int i = 0; i < numOnes; ++i)
    {
        binForm += "1";
    }
    std::stringstream ss;
    ss << std::hex << std::stoll(binForm, NULL, 2);
    std::string out = "0x" + ss.str();
    return out;
}


// https://stackoverflow.com/a/3407254
inline int roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
        return numToRound;

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}

inline std::string removeLastChar(std::string s) {
    if (s.empty() || s.length() == 0) {
        return s;
    }
    return s.substr(0, s.length()-1);
}

inline std::string removeLastThreeChar(std::string s) {
    if (s.empty() || s.length() == 0) {
        return s;
    }
    return s.substr(0, s.length()-3);
}

inline int intPow(int x, unsigned int p) {
    if (p == 0) {
        return 1;
    }
    if (p == 1) {
        return x;  
    } 
  
    int half = intPow(x, p/2);
    if (p % 2 == 0) {
        return half * half;
    } else {
        return x * half * half;
    } 
}

template <typename Out>
inline void split(const std::string &s, char delim, Out result) {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


inline std::string CombineHeader(std::vector<std::string> vec) {
    std::string ret = "";
    if (vec.size() != 0) {
        for (int i=0; i<vec.size(); i++) {
            if (i==0) {
                if (vec[i].back() == '*') {
                    std::vector<std::string> nameAndCount = split(vec[i], '*');
                    ret += (nameAndCount[0]+nameAndCount[1]);
                } else {
                    ret += vec[0];    
                }
            } else {
                if (vec[i].back() == '*') {
                    std::vector<std::string> nameAndCount = split(vec[i], '*');
                    ret += ("_"+nameAndCount[0]+nameAndCount[1]);
                } else {
                    ret += ("_"+vec[i]);
                }
            }
        }
    }
    return ret;
}


#endif
