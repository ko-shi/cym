#ifndef CYM_CODE_GENERATOR
#define CYM_CODE_GENERATOR

#include<cym/CymBase.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/vm/CymOpcode.hpp>
#include<cym/lang/CymInstancedType.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>
#include<cym/utils/CymDoubleKeyMap.hpp>

namespace cym {
	class CodeGen {
		ByteCode byte_code_;
		FuncDef ast_;
		Set<InstancedType> inst_type_;
		DoubleKeyMap<FuncIdentifier,ByteCodeFunc> defined_operators_;
		CodeGen(FuncDef &&a) : ast_(std::move(a)) {
			// primitive type
			inst_type_.emplace(VoidType());
			inst_type_.emplace(IntType());
			const auto int_type = findType(IntType());

			addOperator(u"+", FunctionType{ int_type,{int_type,int_type} }, ByteCodeFunc{ {
					OpUnion(OpPushVariable{ 0 }),
					OpUnion(OpPushVariable{ 1 }),
					OpUnion(OpBinaryOp{BinOp::PLUS}),
					OpUnion(OpCall{}),
					OpUnion(OpReturnBinaryOp{})
				} ,2});
		}
		template<class T>
		InstancedType* findType(T && type) {
			return &inst_type_.emplace(std::forward<T>(type)).first;
		}
		ByteCodeFunc& addOperator(const Str &name, const FunctionType &type_decl, const ByteCodeFunc &code) {
			const auto ret = inst_type_.emplace(std::make_shared<InstancedType>(type_decl));
			defined_operators_.emplace(FuncIdentifier(name, *ret.first),code);
			return defined_operators_[FuncIdentifier(name, *ret.first)];
		}
		FuncDef* findFunc(const Vector<Scope> &s,const Str &name) {
			// TODO: オーバーロード対応
			for (auto i = s.rbegin(); i != s.rend(); i++) {
				if (i->index() == ScopeKind::FUNC_SCOPE) {
					const auto &func = std::get<ScopeKind::FUNC_SCOPE>(*i);
					if (func->inner_func.count(name)) {
						return &func->inner_func[name];
					}
				}
			}
		}
		void generate() {
			const auto void_type = findType(VoidType());
			doGenerate(ast_, {&ast_},addOperator(u"main@", FunctionType{ void_type,{} }, {}));
		}
		InstancedType* doGenerate(const FuncDef &ast, Vector<Scope> stratum, ByteCodeFunc &code) {// copy cnstruct
			for (const auto &i : ast.order) {
				consignAST(i.get(), stratum);
			}
		}
		InstancedType* consignAST(ASTBase *ast,const Vector<Scope> &stratum, ByteCodeFunc &code) {// return value is returrn calue
			switch (ast->id()) {
			case ASTId::CALL_FUNC: {
				const auto func = dynamic_cast<ASTCallFunc*>(ast);
				genCallFunc(stratum, *func, code)
			}
			case ASTId::INFIX: {
				const auto ast = dynamic_cast<ASTInfix*>(i.get());
				ast->
			}
			}
		}
		InstancedType* genCallFunc(const Vector<Scope> &stratum, const ASTCallFunc &func, ByteCodeFunc &code) {

			Vector<InstancedType*> args(func.args.size());
			for (auto &arg : func.args) {
				args.emplace_back(consignAST(arg.get(),stratum,code));
			}
			findFunc(stratum, func.name);
		}
	};
}

#endif