#pragma once

#include <vector>
#include "ResourceManager.h"
#include "MathLib.h"
#include "Light.h"
#include "Model.h"

namespace Bat
{
	class Model;
	class Light;
	class ISceneVisitor;

	class ISceneNode
	{
	public:
		virtual ~ISceneNode() = default;

		virtual void SetName( const std::string& name ) = 0;
		virtual std::string GetName() const = 0;

		virtual ISceneNode* GetParentNode() = 0;
		virtual std::vector<ISceneNode*> GetChildNodes() = 0;
		virtual void AddChildNode( std::unique_ptr<ISceneNode> pNode ) = 0;
		virtual bool RemoveChildNode( ISceneNode* pNode ) = 0;

		virtual size_t GetModelCount() const = 0;
		virtual Model* GetModel( size_t index ) = 0;
		virtual const Model* GetModel( size_t index ) const = 0;
		virtual Model* AddModel( std::vector<Resource<Mesh>> pMeshes ) = 0;
		virtual void RemoveModel( size_t index ) = 0;

		virtual size_t GetLightCount() const = 0;
		virtual Light* GetLight( size_t index ) = 0;
		virtual const Light* GetLight( size_t index ) const = 0;
		virtual Light* AddLight() = 0;
		virtual void RemoveLight( size_t index ) = 0;

		virtual DirectX::XMMATRIX GetTransform() const = 0;
		virtual void SetTransform( const DirectX::XMMATRIX& transform ) = 0;

		virtual void AcceptVisitor( const DirectX::XMMATRIX& transform, ISceneVisitor& visitor ) = 0;
	};

	class BasicSceneNode : public ISceneNode
	{
	public:
		BasicSceneNode( const DirectX::XMMATRIX& transform = DirectX::XMMatrixIdentity(), ISceneNode* pParent = nullptr );
		BasicSceneNode( BasicSceneNode&& ) = default;

		virtual void SetName( const std::string& name ) { m_szName = name; }
		virtual std::string GetName() const { return m_szName; }

		virtual ISceneNode* GetParentNode() override;
		virtual std::vector<ISceneNode*> GetChildNodes() override;
		virtual void AddChildNode( std::unique_ptr<ISceneNode> pNode ) override;
		virtual bool RemoveChildNode( ISceneNode* pNode ) override;

		virtual size_t GetModelCount() const override;
		virtual Model* GetModel( size_t index ) override;
		virtual const Model* GetModel( size_t index ) const override;
		virtual Model* AddModel( std::vector<Resource<Mesh>> pMeshes ) override;
		virtual void RemoveModel( size_t index ) override;

		virtual size_t GetLightCount() const override;
		virtual Light* GetLight( size_t index ) override;
		virtual const Light* GetLight( size_t index ) const override;
		virtual Light* AddLight() override;
		virtual void RemoveLight( size_t index ) override;

		virtual DirectX::XMMATRIX GetTransform() const override;
		virtual void SetTransform( const DirectX::XMMATRIX& transform ) override;

		virtual void AcceptVisitor( const DirectX::XMMATRIX& transform, ISceneVisitor& visitor ) override;
	private:
		ISceneNode* m_pParentNode;
		DirectX::XMMATRIX m_matTransform;
		std::vector<std::unique_ptr<ISceneNode>> m_pChildNodes;
		std::vector<std::unique_ptr<Model>> m_pModels;
		std::vector<std::unique_ptr<Light>> m_pLights;
		std::string m_szName;
	};

	class ISceneVisitor
	{
	public:
		virtual ~ISceneVisitor() = default;

		virtual void Visit( const DirectX::XMMATRIX& transform, ISceneNode& node ) {};
	};

	class SceneGraph
	{
	public:
		SceneGraph() = default;
		SceneGraph( std::unique_ptr<ISceneNode> node );

		void SetRootNode( std::unique_ptr<ISceneNode> node );
		ISceneNode* GetRootNode();
		const ISceneNode* GetRootNode() const;
		void AcceptVisitor( ISceneVisitor& visitor );

		// Gets the current active camera if there is one, otherwise nullptr
		Camera* GetActiveCamera() const { return m_pCamera; }
		// Sets the current active camera. Set to nullptr to remove camera
		void SetActiveCamera( Camera* pCamera ) { m_pCamera = pCamera; }

		ISceneNode* operator->();
		const ISceneNode* operator->() const;
	private:
		std::unique_ptr<ISceneNode> m_pRootNode = std::make_unique<BasicSceneNode>();
		Camera* m_pCamera = nullptr;
	};
}