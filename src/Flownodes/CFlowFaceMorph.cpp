/* Animation_Plugin - for licensing and copyright see license.txt */

#include <StdAfx.h>
#include <CPluginAnimation.h>
#include <Nodes/G2FlowBaseNode.h>

#include <IEntitySystem.h>
#include <IActorSystem.h>
#include <IFacialAnimation.h>
#include <ICryAnimation.h>

#include <map>

namespace AnimationPlugin
{
    template<size_t tnMorphCount = 8>
    class CFlowFaceMorphNode :
        public CFlowBaseNode<eNCT_Instanced>
    {
            enum EInputPorts
            {
                EIP_Start = 0,
                EIP_End = tnMorphCount * 2,
                EIP_Name = 0,
                EIP_Weight,
            };

            enum EOutputPorts
            {
                EOP_Started = 0,
                EOP_Start = 0,
                EOP_End = tnMorphCount,
            };

            int m_nChannelId;
            std::map<IFacialEffector*, float> m_Weights;

            PodArray<IFacialEffector*> m_podEffectors;
            PodArray<float> m_podWeights;
            PodArray<float> m_podBalances;

        public:
            CFlowFaceMorphNode( SActivationInfo* pActInfo )
                : m_podWeights( tnMorphCount ), m_podEffectors( tnMorphCount ), m_podBalances( tnMorphCount )
            {
                m_nChannelId = -1;
            }

            virtual ~CFlowFaceMorphNode( void )
            {
            }

            virtual void Serialize( SActivationInfo* pActInfo, TSerialize ser )
            {
            }

            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                static SInputPortConfig inputs[EIP_End + 1];
                static SOutputPortConfig outputs[EOP_End + 1];
                static string sNames[EOP_End + 1];
                static string sWeights[EOP_End + 1];

                if ( sNames[0].empty() )
                {
                    if ( tnMorphCount > 1 )
                    {
                        for ( int n = EOP_Start; n < EOP_End; ++n )
                        {
                            sNames[n].Format( "Name_%d", n );
                            sWeights[n].Format( "Weight_%d", n );
                        }
                    }

                    else
                    {
                        sNames[0] = "Name";
                        sWeights[0] = "Weight";
                    }
                }

                for ( int n = EIP_Start; n < EIP_End; n += 2 )
                {
                    inputs[n] = InputPortConfig<string>( sNames[n / 2].c_str(), _HELP( "Morph Name" ) );
                    inputs[n + 1] = InputPortConfig<float>( sWeights[n / 2].c_str(), 1.0f, _HELP( "Morph Weight" ) );
                }

                inputs[EIP_End] = InputPortConfig_Null();

                for ( int n = EOP_Start; n < EOP_End; ++n )
                {
                    outputs[n] = OutputPortConfig<float>( sWeights[n].c_str(), _HELP( "Morph Weight" ) );;
                }

                outputs[EOP_End] = OutputPortConfig_Null();

                // Fill in configuration
                config.pInputPorts = inputs;
                config.pOutputPorts = outputs;
                config.sDescription = _HELP( "Performs facial morph" );

                config.SetCategory( EFLN_APPROVED );
                config.nFlags |= EFLN_TARGET_ENTITY;
            }

            virtual void ProcessEvent( EFlowEvent event, SActivationInfo* pActInfo )
            {
                switch ( event )
                {
                    case eFE_Initialize:
                        {
                        }
                        break;

                    case eFE_SetEntityId:
                        {
                            m_Weights.clear();
                        }
                        break;

                    case eFE_Activate:
                        {
                            ICharacterInstance* pCharacter = NULL;
                            IFacialInstance* pFacialInstance = NULL;
                            IFacialModel* pFacialModel = NULL;
                            IFaceState* pFaceState = NULL;
                            IFacialEffectorsLibrary* pLibrary = NULL;
                            IFacialEffector* pEffector = NULL;

                            bool bChanged = false;

                            if ( pActInfo->pEntity )
                            {
                                pCharacter = pActInfo->pEntity->GetCharacter( 0 );

                                if ( pCharacter )
                                {
                                    pFacialInstance = pCharacter->GetFacialInstance();

                                    if ( pFacialInstance )
                                    {
                                        pFacialModel = pFacialInstance->GetFacialModel();
                                        pFaceState = pFacialInstance->GetFaceState();

                                        if ( pFacialModel && pFaceState )
                                        {
                                            pLibrary = pFacialModel->GetLibrary();

                                            if ( pLibrary )
                                            {
                                                for ( int n = EIP_Start; n < EIP_End; n += 2 )
                                                {
                                                    if ( IsPortActive( pActInfo, n + 1 ) )
                                                    {
                                                        string sEffector = GetPortString( pActInfo, n );

                                                        if ( !sEffector.empty() )
                                                        {
                                                            if ( sEffector.at( 0 ) == '#' )
                                                            {
                                                                sEffector = sEffector.substr( 1 );
                                                            }
                                                        }

                                                        if ( !sEffector.empty() )
                                                        {
                                                            pEffector = pLibrary->Find( sEffector );

                                                            if ( pEffector )
                                                            {
                                                                m_Weights[pEffector] = GetPortFloat( pActInfo, n + 1 );
                                                                ActivateOutput<float>( pActInfo, n,  m_Weights[pEffector] );
                                                                bChanged = true;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if ( bChanged && pFacialInstance )
                            {
                                m_podEffectors.clear();
                                m_podWeights.clear();
                                m_podBalances.clear();

                                for ( auto iter = m_Weights.begin(); iter != m_Weights.end(); ++iter )
                                {
                                    m_podEffectors.push_back( iter->first );
                                    m_podWeights.push_back( iter->second );
                                    m_podBalances.push_back( 0.0f );
                                }

                                pFacialInstance->PreviewEffectors( m_podEffectors.GetElements(), m_podWeights.GetElements(), m_podBalances.GetElements(), m_podEffectors.size() );
                            }
                        }

                        break;
                }
            }

            virtual void GetMemoryUsage( ICrySizer* s ) const
            {
                s->Add( *this );
            }

            virtual IFlowNodePtr Clone( SActivationInfo* pActInfo )
            {
                return new CFlowFaceMorphNode( pActInfo );
            }
    };
}

REGISTER_FLOW_NODE_EX( "Animation_Plugin:Facial:Morph", AnimationPlugin::CFlowFaceMorphNode<1>, CFlowFaceMorphNode );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Facial:Morph8", AnimationPlugin::CFlowFaceMorphNode<8>, CFlowFaceMorphNode8 );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Facial:Morph16", AnimationPlugin::CFlowFaceMorphNode<16>, CFlowFaceMorphNode16 );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Facial:Morph24", AnimationPlugin::CFlowFaceMorphNode<24>, CFlowFaceMorphNode24 );
