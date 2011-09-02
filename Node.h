SubjectType(Node)
{
protected:
	Relay *left, *right, *node0, *node1;

public:
	FirstAction(Node,Init)

	Action Init(ConObj *con);
	Action Wakeup(NumObj *num);
	Action BuildDeBruijn(IdObj *id);
	Action Insert(NumObj *num);
	Action Delete(NumObj *key);
	Action LookUp(NumObj *key);
	Action Join(IdObj *id);
	Action Leave(IdObj *id);
};

Action Node::Init(ConObj *con) {

}

Action Node::Wakeup(NumObj *num) {

}

Action Node::BuildDeBruijn(IdObj *id) {

}

Action Node::Insert(NumObj *num) {

}

Action Node::Delete(NumObj *key) {

}

Action Node::LookUp(NumObj *key) {

}

Action Node::Join(IdObj *id) {

}


Action Node::Leave(IdObj *id) {

}
