#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H
#include <string>

class GlobalSettings
{
public:
    static GlobalSettings& instance() {
        static GlobalSettings instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
        return instance;
    }

    int getIter() const { return iter; }
    void setIter(int newValue) { iter = newValue; }
    bool getMode() const {return mode;}
    void setMode(bool newMode) {mode = newMode;}
    bool getEnc() const {return enc;}
    void setEnc(bool newenc) {enc = newenc;}
    bool getCstIter() const {return cstiter;}
    void setCstIter(bool newcstiter) {cstiter = newcstiter;}
    bool getCstHash() const {return csthash;}
    void setCstHash(bool newcsthash) {csthash = newcsthash;}
    std::string getHash() const {return hash;}
    void setHash(std::string newhash){hash = newhash;}
    std::string getEncalg() const{return encalg;}
    void setEncalg(std::string newencalg){encalg = newencalg;}
    // Delete copy constructor and assignment operator
    GlobalSettings(GlobalSettings const&) = delete;
    void operator=(GlobalSettings const&) = delete;

private:
    GlobalSettings() {} // Constructor? (the {} brackets) are needed here.
    // The global state
    int iter = 100000;
    bool mode = true;
    bool enc = true;
    bool cstiter = false;
    bool csthash = false;
    std::string hash = "SHA256";
    std::string encalg = "AES256-GCM";
};

#endif // GLOBALSETTINGS_H
