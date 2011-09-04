#ifndef NODE_H_
#define NODE_H_

SubjectType(Node)
{
protected:
    Relay *left, *right, *node0, *node1;
    Relay *in;
    int id;
    bool isReal;

    void checkDead(Relay *side);
    void checkValid();


public:
    FirstAction(Node,Init)

    Action Init(ConObj *con);
    Action RegisterChiled(IdObj *id);
    Action Wakeup(NumObj *num);
    Action BuildDeBruijn(IdObj *id);
    Action Insert(NumObj *num);
    Action Delete(NumObj *key);
    Action LookUp(NumObj *key);
    Action Join(IdObj *id);
    Action Leave(IdObj *id);
};

Action Node::Init(ConObj *con)
{
    // Unpack constructor object:
    id = con->position;
    isReal = con->isReal;
    delete con;

    // Initialize pointers and relays
    left = NULL;
    right = NULL;
    in = new Relay;
    // Connect to supervisor or real node:
    if (isReal) {
        idp = new IdPair(new IdObj(id, new Identity(in)),
                         new IdObj(id, new Identity(in)));
        parent->call(Supervisor::SetLink, idp);
    } else {
        ido = new IdObj(id, new Identity(in));
        parent->call(Node::RegisterChild, ido);
    }
}

Action Node::Wakeup(NumObj *num)
{
    // Periodically activate BuildDeBruijn:
    if (num->num == 0) {
        BuildDeBruijn(NULL);
    } else {
        num->num--;
        call(Node::Wakeup, num);
    }
}

void Node::checkDead(Relay *side) {
    if (side != NULL && outdeg(side->out) == 0) {
        delete side->out;
        delete side;
        side = NULL;
    }
}

void Node::checkValid() {
    IdObj *tempido;
    if (left != NULL && left->id > id) {
        tempido = new IdObj(left->id, extractIdentity(left->out));
        delete left;
        left = NULL;
        call(Node::BuildList, tempido);
    }
    if (right != NULL && right->id < id) {
        tempido = new IdObj(right->id, extractIdentity(right->out));
        delete right;
        right = NULL;
        call(Node::BuildList, tempido);
    }
}

Action Node::BuildDeBruijn(IdObj *id)
{
    if (isReal && ido == NULL) {
        // Probing:
        if (left != NULL) {
            // TODO: Probing to the left side for node0.
        }
        if (right != NULL) {
            // TODO: Probing to the right side for node1
        }
    }

    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(left);
    checkDead(right);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();
}

Action Node::Insert(NumObj *num)
{

}

Action Node::Delete(NumObj *key)
{

}

Action Node::LookUp(NumObj *key)
{

}

Action Node::Join(IdObj *id)
{

}


Action Node::Leave(IdObj *id)
{

}

#endif  // NODE_H_
