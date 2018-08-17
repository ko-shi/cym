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


			addOperator(u"+", nullptr, FunctionType{ int_type,{int_type,int_type} }, ByteCodeFunc{ {
					OpUnion(OpPushVariable{ 0 }),
					OpUnion(OpPushVariable{ 1 }),
					OpUnion(OpBinaryOp{BinOp::PLUS}),
					OpUnion(OpCall{}),
					OpUnion(OpReturnBinaryOp{})
				} ,2});
		}
		Vector<Size> proposeFunction(const Str &name,const Vector<InstancedType*> &args) {
			Vector<Size> list;
			for (const auto &func : defined_operators_.link) {
				if (func.first.name == name && std::get<FunctionType>(func.first.type->type).args == args) {
					list.push_back(func.second);
				}
			}
			return list;
		}
		InstancedType* makeFuncTypeFromList() {

		}
		template<class T>
		InstancedType* findType(T && type) {
			return &inst_type_.emplace(std::forward<T>(type)).first;
		}
		ByteCodeFunc& addOperator(const Str &name, const FuncDef* def, const FunctionType &type_decl, const ByteCodeFunc &code) {
			const auto ret = inst_type_.emplace(std::make_shared<InstancedType>(type_decl));
			defined_operators_.emplace(FuncIdentifier(name, def, &*ret.first),code);
			return defined_operators_[FuncIdentifier(name, def, &*ret.first)];
		}
		const FuncDef& findFunc(const Vector<Scope> &s,const Str &name, const Vector<InstancedType*> &arg_types) {
			// TODO: オーバーロード対応
			for (auto i = s.rbegin(); i != s.rend(); i++) {
				if (i->index() == ScopeKind::FUNC_SCOPE) {
					const auto &func = std::get<ScopeKind::FUNC_SCOPE>(*i);
					if (func->inner_func.count(name)) {
						const auto &found = func->inner_func[name];
						for (const auto &arg : found.args) {
							//arg.second
						}
						return func->inner_func[name];
					}
				}
			}
		}
		void generate() {
			const auto void_type = findType(VoidType());
			const auto main = doGenerate(ast_, { &ast_ });
			//addOperator(u"main@", FunctionType{ void_type,{} });
		}
		Pair<InstancedType*,ByteCode> doGenerate(const FuncDef &ast, Vector<Scope> stratum) {// copy cnstruct
			for (const auto &i : ast.order) {
				consignAST(i.get(), stratum);
			}
		}
		Pair<InstancedType*,ByteCodeFunc> consignAST(ASTBase *ast,const Vector<Scope> &stratum) {// return value is returrn calue
			switch (ast->id()) {
			case ASTId::CALL_FUNC: {
				const auto func = dynamic_cast<ASTCallFunc*>(ast);
				genCallFunc(stratum, *func);
			}
			case ASTId::INFIX: {
				const auto infix = dynamic_cast<ASTInfix*>(ast);
				infix->l
			}
			}
		}
		Pair<InstancedType*,ByteCodeFunc> genCallFunc(const Vector<Scope> &stratum, const ASTCallFunc &ast) {
			ByteCodeFunc code{};
			Vector<InstancedType*> args(ast.args.size());
			Vector<ByteCodeFunc> ops(ast.args.size());
			for (auto &arg : ast.args) {
				const auto arg_code = consignAST(arg.get(), stratum);
				args.emplace_back(arg_code.first);
				ops.emplace_back(arg_code.second);
			}
			
			//defined_operators_[FuncIdentifier{n}]

			const auto &func = findFunc(stratum, ast.name);
			func.
		}
	};
}

#endif