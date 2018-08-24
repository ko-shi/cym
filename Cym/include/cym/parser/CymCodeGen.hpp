#ifndef CYM_CODE_GENERATOR
#define CYM_CODE_GENERATOR

#include<cym/CymBase.hpp>
#include<cym/lang/CymAST.hpp>
#include<cym/vm/CymOpcode.hpp>
#include<cym/lang/CymInstancedType.hpp>
#include<cym/utils/CymForwardIndexMap.hpp>
#include<cym/utils/CymDoubleKeyMap.hpp>

namespace cym {
	struct InstancedFunction {
		InstancedType* ret_type;
		Vector<InstancedType*> param_type;
		ByteCodeFunc code;
		InstancedFunction() = default;
		InstancedFunction(InstancedFunction&&) = default;
		InstancedFunction(const InstancedFunction&) = default;
		InstancedFunction(InstancedType* ret_type,Vector<InstancedType*> &&param_type,ByteCodeFunc &&code) : ret_type(ret_type),param_type(param_type),code(code){

		}
	};

	class CodeGen {
		ByteCode byte_code_;
		Map<InstancedType,Str> inst_type_;

		DoubleKeyMap<FuncIdentifier,InstancedFunction> defined_operators_;

		ClassDef main_class_;

		CodeGen(ClassDef &&def) : main_class_(std::move(def)) {
			// primitive type
			inst_type_.emplace(u"Void", VoidType());
			inst_type_.emplace(u"Int", IntType());
			const auto int_type = findType(IntType());

			const auto primitive_plus = &main_class_.member.equal_range(u"+").first->second;

			addOperator(FuncIdentifier(primitive_plus, { int_type,int_type }), int_type, {int_type,int_type}, ByteCodeFunc{ {
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

		void addOperator(const FuncIdentifier &funcid, InstancedType* ret_type, Vector<InstancedType*> &&param_type,ByteCodeFunc &&code) {
			defined_operators_.emplace(funcid, InstancedFunction(ret_type,std::move(param_type),std::move(code)));
		}
		const Vector<ClassDef*> findFunc(const Vector<ClassDef*> &stratum,const Str &name, const Vector<InstancedType*> &arg_types) {
			// TODO: オーバーロード対応
			auto list = Vector<ClassDef*>();
			for (auto i = stratum.rbegin(); i != stratum.rend(); i++) {
				const auto found = i->member.equal_range(name);
				std::transform(found.first, found.second, list, [](auto a) { return &a.second; });
			}
			return list;
		}
		void generate() {
			const auto void_type = findType(VoidType());
			const auto main = ;
			//addOperator(u"main@", FunctionType{ void_type,{} });
		}
		Pair<InstancedType*,ByteCode> generateFunction(const FuncIdentifier &funcid, Vector<ClassDef*> stratum) {// copy cnstruct
			Vector<InstancedType*> param_type(funcid.def->param_id.index);
			std::copy(funcid.arg_type.begin(), funcid.arg_type.end(), param_type);

			for(const auto sentence : )
		}
		Pair<InstancedType*,ByteCodeFunc> consignAST(ASTBase *ast,const Vector<ClassDef*> &stratum) {// return value is returrn calue
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
		Vector<Pair<InstancedType*,ByteCodeFunc>> genCallFunc(const Vector<ClassDef*> &stratum, const ASTCallFunc &ast) {
			Vector<InstancedType*> args(ast.args.size());
			Vector<ByteCodeFunc> ops(ast.args.size());
			for (auto &arg : ast.args) {
				const auto arg_code = consignAST(arg.get(), stratum);
				args.emplace_back(arg_code.first);
				ops.emplace_back(arg_code.second);
			}
			
			//defined_operators_[FuncIdentifier{n}]

			auto list = Vector<Pair<InstancedType*, ByteCodeFunc>>();
			const auto func = findFunc(stratum, ast.name, args);
			for (const auto f : func) {
				const auto id = FuncIdentifier(, args);
				if (defined_operators_.link.count(id)) {
					defined_operators_[id]
				}
				else {

				}
			}
		}
	};
}

#endif