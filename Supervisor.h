SubjectType(Supervisor){
    protected:
        int count;
        int total;
        Relay* ListNode;
        IdObj* StartID;

    public:
        FirstAction(Supervisor, Init)
            Action Init(NumObj *num);
        Action SetLink(IdPair *id);
        Action Wakeup(NumObj *num);
};

Action Supervisor::Init(NumObj *num) {

}
