ObjectType(NumObj) {
    public:
        int num;

        NumObj(int i){num=i;}
};

ObjectType(BoolObj) {
    public:
        bool value;

        BoolObj(bool v){value=v;}
};

ObjectType(IdObj) {
    public:
        int num;
        Identity *id;

        IdObj(int i, Identity *d){num=i;  id=d;}
};

ObjectType(ConObj) {
    public:
        bool isReal;
        int position;

        ConObj(BoolObj *r, NumObj *pos) {
            isReal = r->value;
            position = pos->num;
            delete r;
            delete pos;
        }
}
