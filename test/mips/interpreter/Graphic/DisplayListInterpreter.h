#ifndef DISPLAYLISTINTERPRETER_H_
#define DISPLAYLISTINTERPRETER_H_

class DisplayListInterpreter {
public:
	DisplayListInterpreter();
	virtual ~DisplayListInterpreter();

	virtual void Interprete() = 0;
};

#endif /* DISPLAYLISTINTERPRETER_H_ */
