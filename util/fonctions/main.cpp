#include <iostream>
#include <string>

std::string add1(std::string& value){
    bool find = false;
    std::string::size_type i = 0;
    int iValue = 1;

    while(!find && i < value.length()){
        char c = value[i];

        if(!isalpha(c)){ //check si le caractère est un chiffre
            iValue = std::stoi(value.substr(i, value.length()));
            value.erase(i , value.length());
            find = true;
        }
        ++i;
    }

    if(find){ //si un nombre a été trouvé, on fait +1.
        return value + std::to_string(iValue+1);
    } else { //sinon, on rajoute 1
        return value + std::to_string(iValue);
    }
}

int main() {
    
    std::string msg = "text321";

    msg = add1(msg);

    std::cout << msg << std::endl;

    std::string msg2 = "salut98342";
    msg2 = add1(msg2);

    std::cout << msg2 << std::endl;

    return 0;
}