#include<iostream>
#include<fstream>
#include"Cym.hpp"
#include"CymNumConverter.hpp"
#include"CymHandStack.hpp"
#include"CymTree.hpp"

int main() {

	
	using namespace std::string_literals;
	using namespace cym;
	/*
	std::fstream file("test.cym");

	cym::Cym project;
	std::string str;
	while (std::getline(file, str)) {
		project.addLine(cym::toU16String(str));
	}
	project.compile();


	project.run();
	std::cout << cym::toSJisString(project.showIntermediateCode());
	*/
	/*
	cym::TokenKind kind;
	const auto a = cym::getNumKind(u"-114514.1919g",kind);
	*/
	using namespace cym;
	Tree tree(Str(u"野獣先輩"));
	Tree arr(Tree::ArrayType{});
	arr.addWhenArray(Tree(Str(u"YAJU")));
	arr.addWhenArray(Tree(114514));

	Tree obj0(Tree::ObjectType{});
	obj0.addWhenObject(u"pi", Tree(3.14159265));
	obj0.addWhenObject(u"e", Tree(2.78));
	Tree obj(Tree::ObjectType{});
	obj.addWhenObject(u"いいよ!来いよ!", Tree(Str(u"胸にかけて胸に")));
	obj.addWhenObject(u"入って、どうぞ", std::move(tree));
	obj.addWhenObject(u"✝悔い改めて✝", std::move(arr));
	obj.addWhenObject(u"temp", std::move(obj0));
	std::cout << toSJisString(obj.getJSON());
}