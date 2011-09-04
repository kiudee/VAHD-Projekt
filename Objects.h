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
    int num;
    Identity *id;

    IdObj(int i, Identity *d) {
        num=i;
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
#endif  // OBJECTS_H_
