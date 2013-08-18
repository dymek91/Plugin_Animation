/* Animation_Plugin - for licensing and copyright see license.txt */

#include <StdAfx.h>
#include <Nodes/G2FlowBaseNode.h>
#include <CPluginAnimation.h>
#include <IPluginAnimation.h>

#include <IEntitySystem.h>
#include <IActorSystem.h>
#include <IMovementController.h>
#include <Actor.h>

namespace AnimationPlugin
{
    struct SBoneQuat
    {
        int nJointID;
        QuatT qQuat;

        SBoneQuat()
        {
            nJointID = -1;
            qQuat.SetIdentity();
        }
    };

    template<size_t tnCount = 8>
    class CFlowRotateBone :
        public CFlowBaseNode<eNCT_Instanced>
    {
            enum EInputPorts
            {
                EIP_Active = 0,
                EIP_Slot,
                EIP_Radiant,
                EIP_Start,
                EIP_End = EIP_Start + tnCount * 2,
                EIP_Name = EIP_Start,
                EIP_Rotation,
            };

            int m_nSlot;
            bool m_bRadiant;
            PodArray<SBoneQuat> m_Bones;

            ICharacterInstance* m_pChar;
            ISkeletonPose* m_pPose;
        public:

            CFlowRotateBone( SActivationInfo* pActInfo )
                : m_Bones( tnCount )
            {
                m_nSlot = 0;
                m_bRadiant = false;
                m_pChar = NULL;
                m_pPose = NULL;
            }

            virtual ~CFlowRotateBone( void )
            {
            }

            virtual void Serialize( SActivationInfo* pActInfo, TSerialize ser )
            {
            }

            virtual void GetConfiguration( SFlowNodeConfig& config )
            {
                static SInputPortConfig inputs[EIP_End + 1];
                inputs[EIP_Active] = InputPortConfig<bool>( "Active", true, _HELP( "Reapply the data each frame" ) );
                inputs[EIP_Slot] = InputPortConfig<int>( "Slot", 0, _HELP( "Character Slot of the Bones" ) );
                inputs[EIP_Radiant] = InputPortConfig<bool>( "Radiant", false, _HELP( "Radiant or degrees" ) );

                static string sNames[tnCount];
                static string sHumanNames[tnCount];
                static string sRotations[tnCount];

                if ( sNames[0].empty() )
                {
                    if ( tnCount > 1 )
                    {
                        for ( int n = 0; n < tnCount; ++n )
                        {
                            sNames[n].Format( "bone_Bone_%d", n );
                            sHumanNames[n].Format( "Bone_%d", n );
                            sRotations[n].Format( "Rotation_%d", n );
                        }
                    }

                    else
                    {
                        sNames[0] = "bone_Bone";
                        sHumanNames[0] = "Bone";
                        sRotations[0] = "Rotation";
                    }
                }

                for ( int n = EIP_Start; n < EIP_End; n += 2 )
                {
                    int nBone = ( n - EIP_Start ) / 2;
                    inputs[n] = InputPortConfig<string>( sNames[nBone].c_str(), "", _HELP( "Name of the Bone" ), sHumanNames[nBone].c_str(), _UICONFIG( "ref_entity=entityId" ) );
                    inputs[n + 1] = InputPortConfig<Vec3>( sRotations[nBone].c_str(), _HELP( "Rotation of the Bone" ) );
                }

                inputs[EIP_End] = InputPortConfig_Null();

                // Fill in configuration
                config.pInputPorts = inputs;
                config.pOutputPorts = NULL;
                config.sDescription = _HELP( "Rotates Bones" );
                config.SetCategory( EFLN_APPROVED );
                config.nFlags |= EFLN_TARGET_ENTITY;
            }

            virtual void ProcessEvent( EFlowEvent event, SActivationInfo* pActInfo )
            {
                switch ( event )
                {
                    case eFE_Initialize:
                        {
                            m_pChar = NULL;
                            m_pPose = NULL;
                            m_Bones.Free();
                            m_Bones.resize( tnCount );
                        }
                        break;

                    case eFE_SetEntityId:
                        {
                            m_pChar = NULL;
                            m_pPose = NULL;
                        }
                        break;

                    case eFE_Activate:
                        {
                            pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, GetPortBool( pActInfo, EIP_Active ) );
                            m_bRadiant = GetPortBool( pActInfo, EIP_Radiant );
                            m_nSlot = GetPortInt( pActInfo, EIP_Slot );

                            m_pChar = pActInfo->pEntity ? pActInfo->pEntity->GetCharacter( m_nSlot ) : NULL;
                            m_pPose = m_pChar ? m_pChar->GetISkeletonPose() : NULL;

                            for ( int n = EIP_Start; n < EIP_End; n += 2 )
                            {
                                if ( IsPortActive( pActInfo, n + 1 ) )
                                {
                                    // Affected Bone
                                    int nBone = ( n - EIP_Start ) / 2;
                                    string sBone = GetPortString( pActInfo, n );
                                    m_Bones[nBone].nJointID = m_pPose ? m_pPose->GetJointIDByName( sBone ) : -1;

                                    // New Rotation
                                    Vec3 vRotation = GetPortVec3( pActInfo, n + 1 );
                                    Ang3 aRotation;

                                    if ( m_bRadiant )
                                    {
                                        aRotation.Set( vRotation.x, vRotation.y, vRotation.z );
                                    }

                                    else
                                    {
                                        aRotation.Set( DEG2RAD( vRotation.x ), DEG2RAD( vRotation.y ), DEG2RAD( vRotation.z ) );
                                    }

                                    m_Bones[nBone].qQuat.q = Quat::CreateRotationXYZ( aRotation );
                                }
                            }
                        }
                        break;

                    case eFE_Update:
                        {
                            if ( m_pPose )
                            {
                                for ( int n = 0; n < tnCount; ++n )
                                {
                                    if ( m_Bones[n].nJointID >= 0 )
                                    {
                                        QuatT lQuat = m_pPose->GetRelJointByID( m_Bones[n].nJointID );
                                        lQuat.q = m_Bones[n].qQuat.q;
                                        m_pPose->SetPostProcessQuat( m_Bones[n].nJointID, lQuat );
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
                return new CFlowRotateBone( pActInfo );
            }
    };


    template<size_t tnCount = 8>
    class CFlowRotPosBone :
        public CFlowBaseNode<eNCT_Instanced>
    {
        enum EInputPorts
        {
            EIP_Active = 0,
            EIP_Slot,
            EIP_Radiant,
            EIP_Start,
            EIP_End = EIP_Start + tnCount * 3,
            EIP_Name = EIP_Start,
            EIP_Rotation,
            EIP_Position,
        };

        int m_nSlot;
        bool m_bRadiant;
        PodArray<SBoneQuat> m_Bones;

        ICharacterInstance* m_pChar;
        ISkeletonPose* m_pPose;
    public:

        CFlowRotPosBone( SActivationInfo* pActInfo )
            : m_Bones( tnCount )
        {
            m_nSlot = 0;
            m_bRadiant = false;
            m_pChar = NULL;
            m_pPose = NULL;
        }

        virtual ~CFlowRotPosBone( void )
        {
        }

        virtual void Serialize( SActivationInfo* pActInfo, TSerialize ser )
        {
        }

        virtual void GetConfiguration( SFlowNodeConfig& config )
        {
            static SInputPortConfig inputs[EIP_End + 1];
            inputs[EIP_Active] = InputPortConfig<bool>( "Active", true, _HELP( "Reapply the data each frame" ) );
            inputs[EIP_Slot] = InputPortConfig<int>( "Slot", 0, _HELP( "Character Slot of the Bones" ) );
            inputs[EIP_Radiant] = InputPortConfig<bool>( "Radiant", false, _HELP( "Radiant or degrees" ) );

            static string sNames[tnCount];
            static string sHumanNames[tnCount];
            static string sRotations[tnCount];
            static string sPositions[tnCount];

            if ( sNames[0].empty() )
            {
                if ( tnCount > 1 )
                {
                    for ( int n = 0; n < tnCount; ++n )
                    {
                        sNames[n].Format( "bone_Bone_%d", n );
                        sHumanNames[n].Format( "Bone_%d", n );
                        sRotations[n].Format( "Rotation_%d", n );
                        sPositions[n].Format( "Position_%d", n );
                    }
                }

                else
                {
                    sNames[0] = "bone_Bone";
                    sHumanNames[0] = "Bone";
                    sRotations[0] = "Rotation";
                    sPosition[0] = "Position";
                }
            }

            for ( int n = EIP_Start; n < EIP_End; n += 3 )
            {
                int nBone = ( n - EIP_Start ) / 3;
                inputs[n] = InputPortConfig<string>( sNames[nBone].c_str(), "", _HELP( "Name of the Bone" ), sHumanNames[nBone].c_str(), _UICONFIG( "ref_entity=entityId" ) );
                inputs[n + 1] = InputPortConfig<Vec3>( sRotations[nBone].c_str(), _HELP( "Rotation of the Bone" ) );
                inputs[n + 2] = InputPortConfig<Vec3>( sRotations[nBone].c_str(), _HELP( "Position of the Bone" ) );
            }

            inputs[EIP_End] = InputPortConfig_Null();

            // Fill in configuration
            config.pInputPorts = inputs;
            config.pOutputPorts = NULL;
            config.sDescription = _HELP( "Rotate and Position Bones" );
            config.SetCategory( EFLN_APPROVED );
            config.nFlags |= EFLN_TARGET_ENTITY;
        }

        virtual void ProcessEvent( EFlowEvent event, SActivationInfo* pActInfo )
        {
            switch ( event )
            {
            case eFE_Initialize:
                {
                    m_pChar = NULL;
                    m_pPose = NULL;
                    m_Bones.Free();
                    m_Bones.resize( tnCount );
                }
                break;

            case eFE_SetEntityId:
                {
                    m_pChar = NULL;
                    m_pPose = NULL;
                }
                break;

            case eFE_Activate:
                {
                    pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, GetPortBool( pActInfo, EIP_Active ) );
                    m_bRadiant = GetPortBool( pActInfo, EIP_Radiant );
                    m_nSlot = GetPortInt( pActInfo, EIP_Slot );

                    m_pChar = pActInfo->pEntity ? pActInfo->pEntity->GetCharacter( m_nSlot ) : NULL;
                    m_pPose = m_pChar ? m_pChar->GetISkeletonPose() : NULL;

                    for ( int n = EIP_Start; n < EIP_End; n += 3 )
                    {
                        // Affected Bone
                        int nBone = ( n - EIP_Start ) / 3;
                        string sBone = GetPortString( pActInfo, n );
                        m_Bones[nBone].nJointID = m_pPose ? m_pPose->GetJointIDByName( sBone ) : -1;

                        if ( IsPortActive( pActInfo, n + 1 ) )
                        {
                            // New Rotation
                            Vec3 vRotation = GetPortVec3( pActInfo, n + 1 );
                            Ang3 aRotation;

                            if ( m_bRadiant )
                            {
                                aRotation.Set( vRotation.x, vRotation.y, vRotation.z );
                            }

                            else
                            {
                                aRotation.Set( DEG2RAD( vRotation.x ), DEG2RAD( vRotation.y ), DEG2RAD( vRotation.z ) );
                            }

                            m_Bones[nBone].qQuat.q = Quat::CreateRotationXYZ( aRotation );
                        }

                        // New Position
                        if ( IsPortActive( pActInfo, n + 2 ) )
                        {
                            m_Bones[nBone].qQuat.t = GetPortVec3( pActInfo, n + 2 );
                        }
                    }
                }
                break;

            case eFE_Update:
                {
                    if ( m_pPose )
                    {
                        for ( int n = 0; n < tnCount; ++n )
                        {
                            if ( m_Bones[n].nJointID >= 0 )
                            {
                                m_pPose->SetPostProcessQuat( m_Bones[n].nJointID, m_Bones[n].qQuat );
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
            return new CFlowRotateBone( pActInfo );
        }
    };
}

REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:Rotate", AnimationPlugin::CFlowRotateBone<1>, CFlowRotateBone );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:Rotate8", AnimationPlugin::CFlowRotateBone<8>, CFlowRotateBone8 );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:Rotate16", AnimationPlugin::CFlowRotateBone<16>, CFlowRotateBone16 );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:Rotate24", AnimationPlugin::CFlowRotateBone<24>, CFlowRotateBone24 );

REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:RotPos", AnimationPlugin::CFlowRotPosBone<1>, CFlowRotPosBone );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:RotPos8", AnimationPlugin::CFlowRotPosBone<8>, CFlowRotPosBone8 );
REGISTER_FLOW_NODE_EX( "Animation_Plugin:Bone:RotPos16", AnimationPlugin::CFlowRotPosBone<16>, CFlowRotPosBone16 );
