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

    int getDefIter() const{return defiter;}
    std::string getDefhash() const {return defhash; }

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

    std::string getStealg() const {return stealg;}
    void setStealg(std::string newStealg){stealg = newStealg;}

    bool getJPGLSB() const {return LSBJPG;}
    void setJPGLSB(bool newLSB) {LSBJPG = newLSB;}

    int getBulkmin() const { return bulkmin; }
    void setBulkmin(int newValue2) { bulkmin = newValue2; }

    int getChunks() const { return chunks; }
    void setChunks(int newValue2) { chunks = newValue2; }

    std::string getKDF() const {return kdfa;}
    void setKDF(std::string newKDF){kdfa = newKDF;}


private:
    GlobalSettings() {} // Constructor? (the {} brackets) are needed here.
    // The default PBKDF2 parameters
    int defiter = 100000;
    std::string defhash = "SHA256";

    //The settings parameters
    int iter = 100000;
    bool mode = true;
    bool enc = true;
    bool cstiter = false;
    bool csthash = false;
    std::string hash = "SHA256";
    std::string encalg = "AES256-GCM";
    std::string stealg = "JPG-DCT";
    bool LSBJPG = true;
    int bulkmin = 5; //in KB
    int chunks = 10;//in MB
    std::string kdfa = "PBKDF2";
};

#endif // GLOBALSETTINGS_H
