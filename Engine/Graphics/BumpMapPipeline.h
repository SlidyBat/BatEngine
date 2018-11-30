#pragma once

#include "LightPipeline.h"

namespace Bat
{
	class BumpMappedModel : public IModel
	{
	public:
		BumpMappedModel( const Mesh& mesh );
		BumpMappedModel( std::vector<Mesh> meshes );

		virtual void Draw( IPipeline* pPipeline ) const override;
	private:
		std::vector<Mesh> m_Meshes;
	};

	class BumpMapPipeline : public LightPipeline
	{
	public:
		BumpMapPipeline( const std::wstring& vsFilename, const std::wstring& psFilename );

		void BindParameters( IPipelineParameters* pParameters ) override;
	};
}