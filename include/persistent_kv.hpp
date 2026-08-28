#ifndef PERSISTENT_KV_HPP
#define PERSISTENT_KV_HPP

#include <fstream>
#include <sstream>
#include <string>
#include "hash_table.hpp"

class PersistentKV 
{
private:
    HashTable<std::string, std::string> table_;
    std::string file_name_;

    void write_log(const std::string& operation,
                   const std::string& key,
                   const std::string& value = "") 
    {
        std::ofstream file(file_name_, std::ios::app);

        if (operation == "PUT") 
        {
            file << "PUT " << key << " " << value << "\n";
        } else if (operation == "DEL") 
        {
            file << "DEL " << key << "\n";
        }
    }

    void load_log() {
        std::ifstream file(file_name_);
        std::string line;

        while (std::getline(file, line)) 
        {
            std::istringstream input(line);
            std::string operation;
            std::string key;

            input >> operation >> key;

            if (operation == "PUT") 
            {
                std::string value;
                std::getline(input >> std::ws, value);
                table_.put(key, value);
            } else if (operation == "DEL") 
            {
                table_.remove(key);
            }
        }
    }

public:
    explicit PersistentKV(const std::string& file_name)
        : file_name_(file_name) 
    {
        load_log();
    }

    void put(const std::string& key, const std::string& value) 
    {
        table_.put(key, value);
        write_log("PUT", key, value);
    }

    std::string get(const std::string& key) const 
    {
        return table_.get(key);
    }

    bool contains(const std::string& key) const 
    {
        return table_.contains(key);
    }

    bool remove(const std::string& key) 
    {
        bool removed = table_.remove(key);

        if (removed) 
        {
            write_log("DEL", key);
        }

        return removed;
    }

    std::size_t getSize() const 
    {
        return table_.getSize();
    }
};

#endif