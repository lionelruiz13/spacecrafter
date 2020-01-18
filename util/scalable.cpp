#include <iostream>

class Scalable {
public:
	//! Create and initialise
	Scalable(){
		currentValue = 0.f;
        desiredValue = 0.f;
        updateCoeff = 0.f;
        duration = 1000;
        counter = 0;
	}

	~Scalable() {
		;
	}
	
    //! Increments the internal counter of delta_time ticks
	void update(int delta_ticks) {
        if (!isTransiting)
            return;
        counter+=delta_ticks;
		if (counter>=duration) {
			// Transition is over
			isTransiting = false;
			currentValue = desiredValue;
            counter = 0.f;
			// state = (target_value==max_value) ? true : false;
		} else {
			currentValue = currentValue + updateCoeff * delta_ticks;
		}

    }

	Scalable& operator=(float s){
        desiredValue = s;
        isTransiting = true;
        counter = 0.f;
        updateCoeff = (desiredValue - currentValue) / float(duration);
    }
	
    bool operator==(float s) const {
		return currentValue==s;
	}
	
    operator float() const {
		return currentValue;
	}

	void setDuration(int _duration) {
		duration = _duration;
        counter = 0;
	}

	float getDuration(void) {
        return duration;
    }

    void set(float f){
        currentValue = f;
        desiredValue = f;
        isTransiting = false;
    }

    float value() const {
        return currentValue;
    }

    friend std::ostream & operator << (std::ostream & sortie, const Scalable &s);

protected:
    int duration, counter;
	float currentValue, desiredValue;
    float updateCoeff = 0.f;
    bool isTransiting = false;
};


std::ostream & operator << (std::ostream & sortie, const Scalable &s)
{
    sortie << s.value();
    return sortie;
}


int main()
{
    Scalable a;
    std::cout << "Valeur de a " << a << std::endl;

    a.set(9.f);
    std::cout << "Valeur de a " << a << std::endl; 

    a = 7.0;

    std::cout << "Valeur de a " << a << std::endl; 

    for (int i=0; i<54; i++) {
        a.update(20+rand()%20-10);
        std::cout << "Valeur de a " << a << std::endl;
    }

    std::cout << "Tests de calculs "  << std::endl;

    float b = 4.3;
    std::cout << "Valeur de b " << b << std::endl;
    float c = b+a;
    std::cout << "Valeur de b+a " << c << std::endl;


    // a = 17.0;

    // std::cout << "Valeur de a " << a << std::endl; 

    // for (int i=0; i<21; i++) {
    //     a.update(50+rand()%20-10);
    //     std::cout << "Valeur de a " << a << std::endl;
    // }


    return 0;
}