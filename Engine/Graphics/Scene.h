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

		virtual ISceneNode* GetParentNode() = 0;
		virtual std::vector<ISceneNode*> GetChildNodes() = 0;
		virtual void AddChildNode( std::unique_ptr<ISceneNode> pNode ) = 0;
		virtual bool RemoveChildNode( ISceneNode* pNode ) = 0;

		virtual size_t GetModelCount() = 0;
		virtual Model* GetModel( size_t index ) = 0;
		virtual Model* AddModel( std::vector<Resource<Mesh>> pMeshes ) = 0;
		virtual void RemoveModel( size_t index ) = 0;

		virtual size_t GetLightCount() = 0;
		virtual Light* GetLight( size_t index ) = 0;
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

		virtual ISceneNode* GetParentNode() override;
		virtual std::vector<ISceneNode*> GetChildNodes() override;
		virtual void AddChildNode( std::unique_ptr<ISceneNode> pNode ) override;
		virtual bool RemoveChildNode( ISceneNode* pNode ) override;

		virtual size_t GetModelCount() override;
		virtual Model* GetModel( size_t index ) override;
		virtual Model* AddModel( std::vector<Resource<Mesh>> pMeshes ) override;
		virtual void RemoveModel( size_t index ) override;

		virtual size_t GetLightCount() override;
		virtual Light* GetLight( size_t index ) override;
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
		ISceneNode& GetRootNode();
		void AcceptVisitor( ISceneVisitor& visitor );

		// Gets the current active camera if there is one, otherwise nullptr
		Camera* GetActiveCamera() const { return m_pCamera; }
		// Sets the current active camera. Set to nullptr to remove camera
		void SetActiveCamera( Camera* pCamera ) { m_pCamera = pCamera; }
	private:
		BasicSceneNode m_RootNode;
		Camera* m_pCamera = nullptr;
	};
}