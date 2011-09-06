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

ObjectType(BoolObj)
{
public:
    bool value;

    BoolObj(bool v) {
        value=v;
    }
};

ObjectType(IdObj)
{
public:
    double value;
    Identity *id;

    IdObj(double value, Identity *d) {
        value=value;
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

ObjectType(ConObj)
{
public:
    bool isReal;
    int position;

    ConObj(BoolObj *r, NumObj *pos) {
        isReal = r->value;
        position = pos->num;
        delete r;
        delete pos;
    }
};

ObjectType(NodeRelay)
{
public:
    double value;
    Relay *out;

    NodeRelay(IdObj *ido) {
        value = ido->value;
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
}
#endif  // OBJECTS_H_
