#ifndef OBJECTS_H_
#define OBJECTS_H_
ObjectType(NumObj)
{
public:
    int num;

    NumObj(int i) {
        num=i;
    }
};

ObjectType(DoubleObj){
public:
    double num;

    DoubleObj(double i) {
        num=i;
    }
};

ObjectType(IdObj)
{
public:
    double num;
    Identity *id;

    IdObj(double value, Identity *d) {
        num=value;
        id=d;
    }
};

ObjectType(IdPair)
{
public:
    IdObj *ido1;
    IdObj *ido2;
    IdPair(IdObj *i1, IdObj *i2) {
        ido1=i1;
        ido2=i2;
    }
};

ObjectType(NodeRelay)
{
public:
    double value;
    Relay *out;

    NodeRelay(IdObj *ido) {
        value = ido->num;
        out = new Relay(ido->id);
        delete ido;
    }
};

/**
 * Probe is used for probing. It is the same as IdObj plus a flag witch
 *  indicates the routing direction.
 * @author Simon
 */
ObjectType(Probe){
public:
		int phase;
		double value;
		Identity *id;

	    Probe(double v, Identity *d, int p) {
	        value = v;
	        phase = p;
	        id = d;
	    }
};
/**
 * Data to store at nodes
 */
ObjectType(DateObj){
public:
	int num;
	Object date;

	DateObj(int k , Object d){
		date = d;
		num = k;
	}
};

ObjectType(KeyObj){
public:
	int num;
	Identity *id;

	KeyObj(int k, Identity *d){
		num = k;
		id = d;
	}
};

ObjectType(InsertObj){
public:
	DateObj *dob;
	int round;
	double bound;

	InsertObj(DateObj *d, double b){
		dob = d;
		round = 0;
		bound = b;
	}
};



#endif  // OBJECTS_H_
