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
    NumObj *tempnum;

    for (i = 1; i <= num->num; i++) {
        tempnum = new NumObj(i);
        new(List,tempnum);
    }
    count = 0;
    total = num->num;
    ListNode = new int[total];
    StartID = new int[total];
    delete num;
}

Action Supervisor::SetLink(IDPair *idop)
{
    int i;
    NumObj *numo;

    ListNode[idop->ido1->num] = new Relay(idop->ido1->id);
    delete idop->ido1;
    StartID[idop->ido2->num] = idop->ido2;
    delete idop;
    count++;
    if (count == total) {
        for (i=1; i<=total; i++) {
            numo = new NumObj(5);
            // make sure that node i periodically wakes up
            ListNode[i]->call(List::Wakeup, numo);
            // initially connect node i to nodes 2i and 2i+1
            if (2*i<=total) ListNode[i]->call(List::Insert, StartID[2*i]);
            if (2*i+1<=total) ListNode[i]->call(List::Insert, StartID[2*i+1]);
        }
        // wait 100 rounds till testing Delete or Search
        numo = new NumObj(100);
        call(Supervisor::Wakeup, numo);
    }
}
Action Supervisor::Wakeup(NumObj* numo)
{
    NumObj *searchnum;

    if (numo->num > 0) {
        numo->num--;
        call(Supervisor::Wakeup, numo);
    }
    else {
        // test Delete or Search
        /*
         *ListNode[3]->call(List::Delete, NONE);
         */
        //searchnum = new NumObj(9);
        //ListNode[3]->call(List::Search, searchnum);
    }
}

