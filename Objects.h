#ifndef OBJECTS_H_
#define OBJECTS_H_


enum SearchJobType { INSERT, DELETE, LOOKUP, JOIN };

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

ObjectType(InitObj)
{
public:
    bool isReal;
    double num;

    InitObj(double value, bool real) {
        isReal = real;
        num = value;
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
	Object date;//TODO make datatype consistent with HashMap

	DateObj(int k , Object d){
		date = d;
		num = k;
	}
};
/**
 * @deprecated
 * @see SearchJob, DateObj
 */
ObjectType(KeyObj){
public:
	int num;
	Identity *id;

	KeyObj(int k, Identity *d){
		num = k;
		id = d;
	}
};

/**
 * Parameter for any search operation
 *@author Simon
 */
ObjectType(SearchJob){
public:
	double sid; //mandatory
	DateObj dob; //required for Insert TODO pointer?
	IdObj ido; //required for Lookup, Join TODO pointer?
	int type; //indicates the job type (the operation)
	double bound;
	int round;


	//if overloading not possible: one constructor with all arguments! (unneeded arguments are NULL)
	SearchJob(double s, int t, double b){
		sid = s;
		dob = NULL;
		ido = NULL;
		type = t;
		round = 0;
		bound = b;
	}

	SearchJob(double s, int t, double b, DateObj d){
		sid = s;
		dob = d;
		ido = NULL;
		type = t;
		round = 0;
		bound = b;
	}

	SearchJob(double s, int t, double b, IdObj i){
		sid = s;
		dob = NULL;
		ido = i;
		type = t;
		round = 0;
		bound = b;
	}

};

/**
 * @deprecated
 * @see SearchJob
 */
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
