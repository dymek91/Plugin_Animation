/* Animation_Plugin - for licensing and copyright see license.txt */

#include <IPluginBase.h>

#pragma once

/**
* @brief Animation Plugin Namespace
*/
namespace AnimationPlugin
{
    /**
    * @brief plugin Animation concrete interface
    */
    struct IPluginAnimation
    {
        /**
        * @brief Get Plugin base interface
        */
        virtual PluginManager::IPluginBase* GetBase() = 0;

        // TODO: Add your concrete interface declaration here
    };
};