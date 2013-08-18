/* Animation_Plugin - for licensing and copyright see license.txt */

#include <StdAfx.h>
#include <CPluginAnimation.h>
#include <Nodes/G2FlowBaseNode.h>

#include <IEntitySystem.h>
#include <IActorSystem.h>
#include <IFacialAnimation.h>
#include <ICryAnimation.h>

namespace AnimationPlugin
{
    class CFlowEffectorNode :
        public CFlowBaseNode<eNCT_Instanced>
    {
            enum EInputPorts
            {
                EIP_Start,
                EIP_StopLast,
                EIP_Name,
                EIP_Weight,
                EIP_FadeTime,
                EIP_Duration,
                EIP_Repeat
            };

            enum EOutputPorts
            {
                EOP_Started,
            };

            int m_nChannelId;

        public:
            CFlowEffectorNode( SActivationInfo* pActInfo )
            {
                m_nChannelId = -1;
            }

            virtual ~CFlowEffectorNode( void )
            {
            }

            virtual void Serialize( SActivationInfo* pActInfo, TSerialize ser )
            {
            }

            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                static const SInputPortConfig inputs[] =
                {
                    InputPortConfig_Void( "Start", _HELP( "Start" ) ),
                    InputPortConfig_Void( "StopLast", _HELP( "Stop last" ) ),
                    InputPortConfig<string>( "Name", _HELP( "Morph Name" ) ),
                    InputPortConfig<float>( "Weight", 1.0f, _HELP( "Morph Weight" ) ),
                    InputPortConfig<float>( "FadeDuration", 0.1f, _HELP( "Fade in Seconds" ) ),
                    InputPortConfig<float>( "Duration", 2.0f, _HELP( "Duration in Seconds" ) ),
                    InputPortConfig<int>( "Repeat", 0, _HELP( "Repeat" ) ),
                    InputPortConfig_Null(),
                };

                static const SOutputPortConfig outputs[] =
                {
                    OutputPortConfig<int>( "Started", _HELP( "Started" ) ),
                    OutputPortConfig_Null(),
                };

                // Fill in configuration
                config.pInputPorts = inputs;
                config.pOutputPorts = outputs;
                config.sDescription = _HELP( "Starts an FacialAnimation" );
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

                    case eFE_Activate:
                        {
                            ICharacterInstance* pCharacter = NULL;
                            IFacialInstance* pFacialInstance = NULL;
                            IFacialModel* pFacialModel = NULL;
                            IFacialEffectorsLibrary* pLibrary = NULL;
                            IFacialEffector* pEffector = NULL;

                            if ( IsPortActive( pActInfo, EIP_StopLast ) && pActInfo->pEntity )
                            {
                                if ( m_nChannelId > 0 )
                                {
                                    if ( pActInfo->pEntity )
                                    {
                                        pCharacter = pActInfo->pEntity->GetCharacter( 0 );

                                        if ( pCharacter )
                                        {
                                            pFacialInstance = pCharacter->GetFacialInstance();

                                            if ( pFacialInstance )
                                            {
                                                pFacialInstance->StopEffectorChannel( m_nChannelId, GetPortFloat( pActInfo, EIP_FadeTime ) );
                                                m_nChannelId = -1;
                                            }
                                        }
                                    }
                                }
                            }

                            if ( IsPortActive( pActInfo, EIP_Start ) && pActInfo->pEntity )
                            {
                                string sEffector = GetPortString( pActInfo, EIP_Name );

                                if ( !sEffector.empty() )
                                {
                                    if ( sEffector.at( 0 ) == '#' )
                                    {
                                        sEffector = sEffector.substr( 1 );
                                    }
                                }

                                if ( pActInfo->pEntity && !sEffector.empty() )
                                {
                                    pCharacter = pActInfo->pEntity->GetCharacter( 0 );

                                    if ( pCharacter )
                                    {
                                        pFacialInstance = pCharacter->GetFacialInstance();

                                        if ( pFacialInstance )
                                        {
                                            pFacialModel = pFacialInstance->GetFacialModel();

                                            if ( pFacialModel )
                                            {
                                                pLibrary = pFacialModel->GetLibrary();

                                                if ( pLibrary )
                                                {
                                                    pEffector = pLibrary->Find( sEffector );

                                                    if ( pEffector )
                                                    {
                                                        m_nChannelId = pFacialInstance->StartEffectorChannel( pEffector, GetPortFloat( pActInfo, EIP_Weight ), GetPortFloat( pActInfo, EIP_FadeTime ), GetPortFloat( pActInfo, EIP_Duration ), GetPortInt( pActInfo, EIP_Repeat ) );
                                                        ActivateOutput<float>( pActInfo, EOP_Started, m_nChannelId );
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
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
                return new CFlowEffectorNode( pActInfo );
            }
    };
}

REGISTER_FLOW_NODE_EX( "Animation_Plugin:Facial:Animation", AnimationPlugin::CFlowEffectorNode, CFlowEffectorNode );
