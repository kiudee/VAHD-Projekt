#ifndef OBJECTS_H_
#define OBJECTS_H_

#include <iostream>
#undef delete
#undef new
#include <memory>
#undef delete
#undef new

// Workaround: Macro "new" collides with definition from <memory>
#define new(subject,object) \
    _create((Subject *) this, (Subject *) new subject((Object* &) object))
// Workaround: Macro "delete" collides with definition from <memory>
#define delete(subject) \
    _kill((Subject *) this, subject)

#define DATATYPE std::string

enum SearchJobType { INSERT, DELETE, LOOKUP, JOIN, DATATRANSFER };

ObjectType(NumObj)
{
public:
    int num;

    NumObj(int i) {
        num = i;
    }
};

ObjectType(DoubleObj)
{
public:
    double num;

    DoubleObj(double i) {
        num = i;
    }
};

ObjectType(IdObj)
{
public:
    double num;
    Identity *id;
    int debugID;

    IdObj(double value, Identity * d, int did) {
        num = value;
        id = d;
        debugID = did;
    }

    /*
     *IdObj(const IdObj& ido) : num(ido.num) {
     *    id = new Identity;
     *    *id = *ido.id;
     *}
     */
};

ObjectType(InitObj)
{
public:
    bool isReal;
    double num;
    std::shared_ptr<std::ofstream> csvFile;

    InitObj(double value, bool real, std::shared_ptr<std::ofstream> file) {
        isReal = real;
        num = value;
        csvFile = file;
    }
};

ObjectType(IdPair)
{
public:
    IdObj *ido1;
    IdObj *ido2;
    IdPair(IdObj * i1, IdObj * i2) {
        ido1 = i1;
        ido2 = i2;
    }
};

ObjectType(NodeRelay)
{
public:
    double num;
    Relay *out;
    int debugID;

    NodeRelay(IdObj * ido) {
        num = ido->num;
        out = new Relay(ido->id);
        debugID = ido->debugID;
    }

    NodeRelay(double value, Identity * d) {
        num = value;
        out = new Relay(d);
    }
};

/**
 * Probe is used for probing. It is the same as IdObj plus a flag witch
 *  indicates the routing direction.
 * @author Simon
 */
ObjectType(Probe)
{
public:
    int phase;
    double num;
    Identity *id;

    Probe(double v, Identity * d, int p) {
        num = v;
        phase = p;
        id = d;
    }
};

/**
 * Data to store at nodes
 */
ObjectType(DateObj)
{
public:
    int num;
    DATATYPE date;

    DateObj(int k , DATATYPE d) {
        date = d;
        num = k;
    }
};

/**
 * @deprecated
 * @see SearchJob, DateObj
 */
ObjectType(KeyObj)
{
public:
    int num;
    Identity *id;

    KeyObj(int k, Identity * d) {
        num = k;
        id = d;
    }
};

/**
 * Parameter for any search operation
 *@author Simon
 */
ObjectType(SearchJob)
{
public:
    double sid; //mandatory
    DateObj *dob; //required for Insert
    IdObj *ido; //required for Lookup, Join
    int type; //indicates the job type (the operation)
    double bound;
    int round;
    int hopcount; //is basically the same as round, but round will sometimes be reset to 0.
    int key;
    double startID;

    //TODO make sure sid <= MAX && ido->num <=MAX (?)
    SearchJob(double s, int t, double b) :
        sid(s), dob(NULL), ido(NULL), type(t),
        bound(b), round(0), hopcount(0), key(0),
        startID(0.0)
    { }

    SearchJob(double s, int t, double b, DateObj * d, double startID) :
        sid(s), dob(d), ido(NULL), type(t),
        bound(b), round(0), hopcount(0), key(0),
        startID(startID) {
        /*insert*/
    }

    SearchJob(double s, int t, double b, IdObj * i) :
        sid(s), dob(NULL), ido(i), type(t),
        bound(b), round(0), hopcount(0), key(0),
        startID(0.0) {
        /*join*/
    }

    SearchJob(double s, int t, double b, IdObj * i, int k, double startID) :
        sid(s), dob(NULL), ido(i), type(t),
        bound(b), round(0), hopcount(0), key(k),
        startID(startID) {
        /*lookup*/
    }

    SearchJob(double s, int t, double b, int k, double startID) :
        sid(s), dob(NULL), ido(NULL), type(t),
        bound(b), round(0), hopcount(0), key(k),
        startID(startID) {
        /*delete*/
    }
};

/**
 * @deprecated
 * @see SearchJob
 */
ObjectType(InsertObj)
{
public:
    DateObj *dob;
    int round;
    double bound;

    InsertObj(DateObj * d, double b) {
        dob = d;
        round = 0;
        bound = b;
    }
};

/**
 * Used by supervisor to get to know the virtual nodes.
 */
ObjectType(NodePair)
{
public:
    Subject *node0;
    Subject *node1;

    NodePair(Subject * n0, Subject * n1) {
        node0 = n0;
        node1 = n1;
    }
};

#endif  // OBJECTS_H_
