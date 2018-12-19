#pragma once

#include <vector>
#include "ResourceManager.h"
#include "MathLib.h"

namespace Bat
{
	class Model;
	class ISceneVisitor;

	class ISceneNode
	{
	public:
		virtual ~ISceneNode() = default;

		virtual ISceneNode* GetParentNode() = 0;
		virtual std::vector<ISceneNode*> GetChildNodes() = 0;
		virtual void AddChildNode( std::unique_ptr<ISceneNode> pNode ) = 0;
		virtual bool RemoveChildNode( ISceneNode* pNode ) = 0;

		virtual std::vector<Model*> GetModels() = 0;
		virtual Model* AddModel( const Model& model ) = 0;
		virtual void RemoveModel( Model* pModel ) = 0;

		virtual DirectX::XMMATRIX GetTransform() const = 0;
		virtual void SetTransform( const DirectX::XMMATRIX& transform ) = 0;

		virtual void AcceptVisitor( ISceneVisitor& visitor ) = 0;
	};

	class BasicSceneNode : public ISceneNode
	{
	public:
		BasicSceneNode( const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity(), ISceneNode* pParent = nullptr );
		virtual ~BasicSceneNode() override;

		BasicSceneNode( BasicSceneNode&& ) = default;

		virtual ISceneNode* GetParentNode() override;
		virtual std::vector<ISceneNode*> GetChildNodes() override;
		virtual void AddChildNode( std::unique_ptr<ISceneNode> pNode ) override;
		virtual bool RemoveChildNode( ISceneNode* pNode ) override;

		virtual std::vector<Model*> GetModels() override;
		virtual Model* AddModel( const Model& model ) override;
		virtual void RemoveModel( Model* id ) override;

		virtual DirectX::XMMATRIX GetTransform() const override;
		virtual void SetTransform( const DirectX::XMMATRIX& transform ) override;

		virtual void AcceptVisitor( ISceneVisitor& visitor ) override;
	private:
		ISceneNode* m_pParentNode;
		DirectX::XMMATRIX m_matTransform;
		std::vector<std::unique_ptr<ISceneNode>> m_pChildNodes;
		std::vector<Model*> m_pModels;
	};

	class ISceneVisitor
	{
	public:
		virtual ~ISceneVisitor() = default;

		virtual void Visit( ISceneNode& node ) {};
		virtual void Visit( Model& model ) {};
	};

	class SceneGraph
	{
	public:
		ISceneNode& GetRootNode();
		void AcceptVisitor( ISceneVisitor& visitor );
	private:
		BasicSceneNode m_RootNode;
	};
}