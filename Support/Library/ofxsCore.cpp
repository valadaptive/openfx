/*
  OFX Support Library, a library that skins the OFX plug-in API with C++ classes.
  Copyright (C) 2004 The Foundry Visionmongers Ltd
  Author Bruno Nicoletti bruno@thefoundry.co.uk

  This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with this library; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  The Foundry Visionmongers Ltd
  35-36 Gt Marlborough St 
  London W1V 7FN
  England
*/

#include "./ofxsSupportPrivate.H"

namespace OFX {
    /** @brief the global host description */
    ImageEffectHostDescription *gHostDescription;
  
    /** @brief Throws an @ref OFX::Exception depending on the status flag passed in */
    void
    throwSuiteStatusException(OfxStatus stat) throw(OFX::Exception::Suite, std::bad_alloc)
    {
        switch (stat) {
        case kOfxStatOK :
        case kOfxStatReplyYes :
        case kOfxStatReplyNo :
        case kOfxStatReplyDefault :
            break;
    
        case kOfxStatErrMemory :
            throw std::bad_alloc();

        default :
            throw OFX::Exception::Suite(stat);
        }
    }


    /** @brief maps status to a string */
    char *
    mapStatusToString(OfxStatus stat)
    {
        switch(stat) {    
        case kOfxStatOK             : return "kOfxStatOK";
        case kOfxStatFailed         : return "kOfxStatFailed";
        case kOfxStatErrFatal       : return "kOfxStatErrFatal";
        case kOfxStatErrUnknown     : return "kOfxStatErrUnknown";
        case kOfxStatErrMissingHostFeature : return "kOfxStatErrMissingHostFeature";
        case kOfxStatErrUnsupported : return "kOfxStatErrUnsupported";
        case kOfxStatErrExists      : return "kOfxStatErrExists";
        case kOfxStatErrFormat      : return "kOfxStatErrFormat";
        case kOfxStatErrMemory      : return "kOfxStatErrMemory";
        case kOfxStatErrBadHandle   : return "kOfxStatErrBadHandle";
        case kOfxStatErrBadIndex    : return "kOfxStatErrBadIndex";
        case kOfxStatErrValue       : return "kOfxStatErrValue";
        case kOfxStatReplyYes       : return "kOfxStatReplyYes";
        case kOfxStatReplyNo        : return "kOfxStatReplyNo";
        case kOfxStatReplyDefault   : return "kOfxStatReplyDefault";
        case kOfxStatErrImageFormat : return "kOfxStatErrImageFormat";
        }
        return "UNKNOWN STATUS CODE";
    }
  
    /** @brief map a std::string to a context */
    ContextEnum 
    mapToContextEnum(const std::string &s) throw(std::invalid_argument)
    {
        if(s == kOfxImageEffectContextGenerator) return eContextGenerator;
        if(s == kOfxImageEffectContextFilter) return eContextFilter;
        if(s == kOfxImageEffectContextTransition) return eContextTransition;
        if(s == kOfxImageEffectContextPaint) return eContextPaint;
        if(s == kOfxImageEffectContextGeneral) return eContextGeneral;
        if(s == kOfxImageEffectContextRetimer) return eContextRetimer;
        OFX::Log::error(true, "Unknown image effect context '%s'", s.c_str());
        throw std::invalid_argument(s);
    }
  

    /** @brief OFX::Private namespace, for things private to the support library */
    namespace Private {

        // Suite and host pointers
        OfxHost               *gHost = 0;
        OfxImageEffectSuiteV1 *gEffectSuite = 0;
        OfxPropertySuiteV1    *gPropSuite = 0;
        OfxInteractSuiteV1    *gInteractSuite = 0;
        OfxParameterSuiteV1   *gParamSuite = 0;
        OfxMemorySuiteV1      *gMemorySuite = 0;
        OfxMultiThreadSuiteV1 *gThreadSuite = 0;
        OfxMessageSuiteV1     *gMessageSuite = 0;
  
        /** @brief Creates the global host description and sets its properties */
        void
        fetchHostDescription(OfxHost *host)
        {
            OFX::Log::error(OFX::gHostDescription != 0, "Tried to create host description when we already have one.");
            if(OFX::gHostDescription == 0) {
	
                // make one
                gHostDescription = new ImageEffectHostDescription;

                // wrap the property handle up with a property set
                PropertySet hostProps(host->host);

                // and get some properties
                gHostDescription->hostName                   = hostProps.propGetString(kOfxPropName);
                gHostDescription->hostIsBackground           = hostProps.propGetInt(kOfxImageEffectHostPropIsBackground) != 0;
                gHostDescription->supportsOverlays           = hostProps.propGetInt(kOfxImageEffectPropSupportsOverlays) != 0;
                gHostDescription->supportsMultiResolution    = hostProps.propGetInt(kOfxImageEffectPropSupportsMultiResolution) != 0;
                gHostDescription->supportsTiles              = hostProps.propGetInt(kOfxImageEffectPropSupportsTiles) != 0;
                gHostDescription->temporalClipAccess         = hostProps.propGetInt(kOfxImageEffectPropTemporalClipAccess) != 0;
                gHostDescription->supportsMultipleClipDepths = hostProps.propGetInt(kOfxImageEffectPropSupportsMultipleClipDepths) != 0;
                gHostDescription->supportsMultipleClipPARs   = hostProps.propGetInt(kOfxImageEffectPropSupportsMultipleClipPARs) != 0;
                gHostDescription->supportsSetableFrameRate   = hostProps.propGetInt(kOfxImageEffectPropSetableFrameRate) != 0;
                gHostDescription->supportsSetableFielding    = hostProps.propGetInt(kOfxImageEffectPropSetableFielding) != 0;
                gHostDescription->supportsStringAnimation    = hostProps.propGetInt(kOfxParamHostPropSupportsStringAnimation) != 0;
                gHostDescription->supportsCustomInteract     = hostProps.propGetInt(kOfxParamHostPropSupportsCustomInteract) != 0;
                gHostDescription->supportsChoiceAnimation    = hostProps.propGetInt(kOfxParamHostPropSupportsChoiceAnimation) != 0;
                gHostDescription->supportsBooleanAnimation   = hostProps.propGetInt(kOfxParamHostPropSupportsBooleanAnimation) != 0;
                gHostDescription->supportsCustomAnimation    = hostProps.propGetInt(kOfxParamHostPropSupportsCustomAnimation) != 0;
                gHostDescription->maxParameters              = hostProps.propGetInt(kOfxParamHostPropMaxParameters);
                gHostDescription->maxPages                   = hostProps.propGetInt(kOfxParamHostPropMaxPages);
                gHostDescription->pageRowCount               = hostProps.propGetInt(kOfxParamHostPropPageRowColumnCount, 0);
                gHostDescription->pageColumnCount            = hostProps.propGetInt(kOfxParamHostPropPageRowColumnCount, 1);
            }
        }

        /** @brief fetch the effect property set from the ImageEffectHandle */
        OFX::PropertySet
        fetchEffectProps(OfxImageEffectHandle handle)
        {
            // get the property handle
            OfxPropertySetHandle propHandle;
            OfxStatus stat = OFX::Private::gEffectSuite->getPropertySet(handle, &propHandle);
            throwSuiteStatusException(stat);
            return OFX::PropertySet(propHandle);
        }

        /** @brief Fetch's a suite from the host and logs errors */
        static void *
        fetchSuite(char *suiteName, int suiteVersion, bool optional = false)
        {
            void *suite = gHost->fetchSuite(gHost->host, suiteName, suiteVersion);
            if(optional)
                OFX::Log::warning(suite == 0, "Could not fetch the optional suite '%s' version %d.", suiteName, suiteVersion);
            else
                OFX::Log::error(suite == 0, "Could not fetch the mandatory suite '%s' version %d.", suiteName, suiteVersion);
            if(!optional && suite == 0) throw OFX::Exception::HostInadequate(suiteName);
            return suite;
        }

        /** @brief Keeps count of how many times load/unload have been called */
        int gLoadCount = 0;

        /** @brief Library side load action, this fetches all the suite pointers */
        void loadAction(void)
        {
            OFX::Log::error(gLoadCount != 0, "Load action called more than once without unload being called.");
            gLoadCount++;  
  
            OfxStatus status = kOfxStatOK;
  
            // fetch the suites
            OFX::Log::error(gHost == 0, "Host pointer has not been set.");
            if(!gHost) throw OFX::Exception::Suite(kOfxStatErrBadHandle);
    
            if(gLoadCount == 1) {
                gEffectSuite    = (OfxImageEffectSuiteV1 *) fetchSuite(kOfxImageEffectSuite, 1);
                gPropSuite      = (OfxPropertySuiteV1 *)    fetchSuite(kOfxPropertySuite, 1);
                gParamSuite     = (OfxParameterSuiteV1 *)   fetchSuite(kOfxParameterSuite, 1);
                gMemorySuite    = (OfxMemorySuiteV1 *)      fetchSuite(kOfxMemorySuite, 1);
                gThreadSuite    = (OfxMultiThreadSuiteV1 *) fetchSuite(kOfxMultiThreadSuite, 1);
                gMessageSuite   = (OfxMessageSuiteV1 *)     fetchSuite(kOfxMessageSuite, 1);
      
                // OK check and fetch host information
                fetchHostDescription(gHost);
      
                // fetch the interact suite if the host supports interaction
                if(OFX::gHostDescription->supportsOverlays || OFX::gHostDescription->supportsCustomInteract)
                    gInteractSuite  = (OfxInteractSuiteV1 *)    fetchSuite(kOfxInteractSuite, 1);
            }

            // initialise the validation code
            OFX::Validation::initialise();

            // validate the host
            OFX::Validation::validateHostProperties(gHost);

        }

        /** @brief Library side unload action, this fetches all the suite pointers */
        void
        unloadAction(void)
        {
            gLoadCount--;
            OFX::Log::error(gLoadCount != 0, "UnLoad action called without a corresponding load action having been called.");
  
            // force these to null
            gEffectSuite = 0;
            gPropSuite = 0;
            gParamSuite = 0;
            gMemorySuite = 0;
            gThreadSuite = 0;
            gMessageSuite = 0;
            gInteractSuite = 0;
        }


        /** @brief fetches our pointer out of the props on the handle */
        ImageEffect *retrieveImageEffectPointer(OfxImageEffectHandle handle) 
        {
            ImageEffect *instance;

            // get the prop set on the handle
            OfxPropertySetHandle propHandle;
            OfxStatus stat = OFX::Private::gEffectSuite->getPropertySet(handle, &propHandle);
            throwSuiteStatusException(stat);

            // make our wrapper object
            PropertySet props(propHandle);

            // fetch the instance data out of the properties
            instance = (ImageEffect *) props.propGetPointer(kOfxPropInstanceData);

            OFX::Log::error(instance == 0, "Instance data handle in effect instance properties is NULL!");

            // need to throw something here
      
            // and dance to the music
            return instance;
        }

        /** @brief Checks the handles passed into the plugin's main entry point */
        void
        checkMainHandles(const std::string &action,  const void *handle, 
                         OfxPropertySetHandle inArgsHandle,  OfxPropertySetHandle outArgsHandle,
                         bool handleCanBeNull, bool inArgsCanBeNull, bool outArgsCanBeNull)
        {
            if(handleCanBeNull)
                OFX::Log::warning(handle != 0, "Handle passed to '%s' is not null.", action.c_str());
            else
                OFX::Log::error(handle == 0, "'Handle passed to '%s' is null.", action.c_str());
  
            if(inArgsCanBeNull)
                OFX::Log::warning(inArgsHandle != 0, "'inArgs' Handle passed to '%s' is not null.", action.c_str());
            else
                OFX::Log::error(inArgsHandle == 0, "'inArgs' handle passed to '%s' is null.", action.c_str());
  
            if(outArgsCanBeNull)
                OFX::Log::warning(outArgsHandle != 0, "'outArgs' Handle passed to '%s' is not null.", action.c_str());
            else
                OFX::Log::error(outArgsHandle == 0, "'outArgs' handle passed to '%s' is null.", action.c_str());
  
            // validate the property sets on the arguments
            OFX::Validation::validateActionArgumentsProperties(action, inArgsHandle, outArgsHandle);

            // throw exceptions if null when not meant to be null
            if(!handleCanBeNull && !handle)         throwSuiteStatusException(kOfxStatErrBadHandle);
            if(!inArgsCanBeNull && !inArgsHandle)   throwSuiteStatusException(kOfxStatErrBadHandle);
            if(!outArgsCanBeNull && !outArgsHandle) throwSuiteStatusException(kOfxStatErrBadHandle);
        }


        /** @brief Fetches the arguments used in a render action 'inargs' property set into a POD struct */
        static void
        getRenderActionArguments(RenderArguments &args,  OFX::PropertySet inArgs)
        {
            args.time = inArgs.propGetDouble(kOfxPropTime);

            args.renderScale.x = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 0);
            args.renderScale.y = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 1);

            args.renderWindow.x1 = inArgs.propGetInt(kOfxImageEffectPropRenderWindow, 0);
            args.renderWindow.y1 = inArgs.propGetInt(kOfxImageEffectPropRenderWindow, 1);
            args.renderWindow.x2 = inArgs.propGetInt(kOfxImageEffectPropRenderWindow, 2);
            args.renderWindow.y2 = inArgs.propGetInt(kOfxImageEffectPropRenderWindow, 3);

            std::string str = inArgs.propGetString(kOfxImageEffectPropFieldToRender);
            try {
                args.fieldToRender = mapStrToFieldEnum(str);
            }
            catch (std::invalid_argument &ex) {
                // dud field?
                OFX::Log::error(true, "Unknown field to render '%s'", str.c_str());
	
                // HACK need to throw something to cause a failure
            }
        }

        /** @brief Library side render action, fetches relevant properties and calls the client code */
        void
        renderAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs)
        {
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
            RenderArguments args;
            
            // get the arguments 
            getRenderActionArguments(args, inArgs);

            // and call the plugin client render code
            effectInstance->render(args);
        }

        /** @brief Library side render begin sequence render action, fetches relevant properties and calls the client code */
        void
        beginSequenceRenderAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs)
        {
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
      
            BeginSequenceRenderArguments args;

            args.frameRange.min = inArgs.propGetDouble(kOfxImageEffectPropFrameRange, 0);
            args.frameRange.max = inArgs.propGetDouble(kOfxImageEffectPropFrameRange, 1);

            args.frameStep      = inArgs.propGetDouble(kOfxImageEffectPropFrameStep, 0);

            args.renderScale.x = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 0);
            args.renderScale.y = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 1);

            args.isInteractive = inArgs.propGetInt(kOfxPropIsInteractive);

            // and call the plugin client render code
            effectInstance->beginSequenceRender(args);
        }
        
        /** @brief Library side render begin sequence render action, fetches relevant properties and calls the client code */
        void
        endSequenceRenderAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs)
        {
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
      
            EndSequenceRenderArguments args;

            args.renderScale.x = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 0);
            args.renderScale.y = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 1);

            args.isInteractive = inArgs.propGetInt(kOfxPropIsInteractive);

            // and call the plugin client render code
            effectInstance->endSequenceRender(args);
        }

        /** @brief Library side render begin sequence render action, fetches relevant properties and calls the client code */
        bool
        isIdentityAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs, OFX::PropertySet &outArgs)
        {
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
            RenderArguments args;
            
            // get the arguments 
            getRenderActionArguments(args, inArgs);

            // and call the plugin client isIdentity code
            Clip *identityClip = 0;
            double identityTime = args.time;
            bool v = effectInstance->isIdentity(args, identityClip, identityTime);

            if(v && identityClip) {
                outArgs.propSetString(kOfxPropName, identityClip->name());
                outArgs.propSetDouble(kOfxPropTime, identityTime);                
                return true;
            }
            return false;
        }

        /** @brief Library side get region of definition function */
        bool
        regionOfDefinitionAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs, OFX::PropertySet &outArgs)
        {
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
            RegionOfDefinitionArguments args;

            args.renderScale.x = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 0);
            args.renderScale.y = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 1);

            args.time = inArgs.propGetDouble(kOfxPropTime);

            // and call the plugin client code
            OfxRectD rod;
            bool v = effectInstance->getRegionOfDefinition(args, rod);

            if(v) {
                outArgs.propSetDouble(kOfxImageEffectPropRegionOfDefinition, rod.x1, 0);
                outArgs.propSetDouble(kOfxImageEffectPropRegionOfDefinition, rod.y1, 1);
                outArgs.propSetDouble(kOfxImageEffectPropRegionOfDefinition, rod.x2, 2);
                outArgs.propSetDouble(kOfxImageEffectPropRegionOfDefinition, rod.y2, 3);
                return true;
            }
            return false;
        }

        /** @brief Library side get regions of interest function */
        bool
        regionsOfInterestAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs, OFX::PropertySet &outArgs)
        {
            /** @brief local class to set the roi of a clip */
            class ActualROISetter : public OFX::RegionOfInterestSetter {
                bool doneSomething_;
                OFX::PropertySet &outArgs_;

            public :
                /** @brief ctor */
                ActualROISetter(OFX::PropertySet &args) 
                  : outArgs_(args)
                  , doneSomething_(false) 
                { }
                
                /** @brief did we set something ? */
                bool didSomething(void) const {return doneSomething_;}
                
                /** @brief set the RoI of the clip */
                virtual void setRegionOfInterest(const Clip &clip, OfxRectD &roi)
                {
                    // construct the name of the property
                    std::string propName = "OfxImageClipPropRoI_";
                    propName = propName + clip.name();

                    // and set it
                    outArgs_.propSetDouble(propName, roi.x1, 0);
                    outArgs_.propSetDouble(propName, roi.y1, 1);
                    outArgs_.propSetDouble(propName, roi.x2, 2);
                    outArgs_.propSetDouble(propName, roi.y2, 3);      

                    // and record the face we have done something
                    doneSomething_ = true;
                }
            }; // end of local class
            
            // fetch our effect pointer 
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
            RegionsOfInterestArguments args;

            // fetch in arguments from the prop handle
            args.renderScale.x = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 0);
            args.renderScale.y = inArgs.propGetDouble(kOfxImageEffectPropRenderScale, 1);

            args.regionOfInterest.x1 = inArgs.propGetDouble(kOfxImageEffectPropRegionOfInterest, 0);
            args.regionOfInterest.y1 = inArgs.propGetDouble(kOfxImageEffectPropRegionOfInterest, 1);
            args.regionOfInterest.x2 = inArgs.propGetDouble(kOfxImageEffectPropRegionOfInterest, 2);
            args.regionOfInterest.y2 = inArgs.propGetDouble(kOfxImageEffectPropRegionOfInterest, 3);

            args.time = inArgs.propGetDouble(kOfxPropTime);
            
            // make a roi setter object
            ActualROISetter setRoIs(outArgs);

            // and call the plugin client code
            effectInstance->getRegionsOfInterest(args, setRoIs);

            // did we do anything ?
            if(setRoIs.didSomething()) 
                return true;
            return false;
        }

        
        /** @brief Library side frames needed action */
        bool
        framesNeededAction(OfxImageEffectHandle handle, OFX::PropertySet inArgs, OFX::PropertySet &outArgs)
        {
            /** @brief local class to set the frames needed from a clip */
            class ActualSetter : public OFX::FramesNeededSetter {
                OFX::PropertySet &outArgs_;                                  /**< @brief property set to set values in */
                std::map<std::string, std::vector<OfxRangeD> > frameRanges_;  /**< @brief map holding a bunch of frame ranges, one for each clip */

            public :
                /** @brief ctor */
                ActualSetter(OFX::PropertySet &args) 
                  : outArgs_(args)
                { }
                
                /** @brief set the RoI of the clip */
                virtual void setFramesNeeded(const Clip &clip, const OfxRangeD &range) 
                {
                    // insert this into the vector which is in the map
                    frameRanges_[clip.name()].push_back(range);
                }
                
                /** @brief write frameRanges_ back to the property set */
                bool setOutProperties(void) 
                {
                    bool didSomething = false;

                    std::map<std::string, std::vector<OfxRangeD> >::iterator i;

                    for(i = frameRanges_.begin(); i != frameRanges_.end(); ++i) {
                        didSomething = true;

                        // Make the property name we are setting
                        std::string propName = "OfxImageClipPropFrameRange_";
                        propName = propName + i->first;

                        // fetch the list of frame ranges
                        std::vector<OfxRangeD> &clipRange = i->second;
                        std::vector<OfxRangeD>::iterator j;
                        int n = 0;

                        // and set 'em
                        for(j = clipRange.begin(); j < clipRange.end(); ++j) {
                            outArgs_.propSetDouble(propName, j->min, n++);
                            outArgs_.propSetDouble(propName, j->max, n++);
                        }
                    }

                    return didSomething;
                }

            }; // end of local class
            
            // fetch our effect pointer 
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);
            FramesNeededArguments args;

            // fetch in arguments from the prop handle
            args.time = inArgs.propGetDouble(kOfxPropTime);
            
            // make a roi setter object
            ActualSetter setFrames(outArgs);

            // and call the plugin client code
            effectInstance->getFramesNeeded(args, setFrames);

            // Write it back to the properties and see if we set anything
            if(setFrames.setOutProperties()) 
                return true;
            return false;
        }


        /** @brief Library side get regions of interest function */
        bool
        clipPreferencesAction(OfxImageEffectHandle handle, OFX::PropertySet &outArgs)
        {
            // fetch our effect pointer 
            ImageEffect *effectInstance = retrieveImageEffectPointer(handle);

            // set up our clip preferences setter
            ClipPreferencesSetter prefs(outArgs);
            
            // and call the plugin client code
            effectInstance->getClipPreferences(prefs);

            // did we do anything ?
            if(prefs.didSomething()) 
                return true;
            return false;
        }

        /** @brief The main entry point for the plugin
        */
        OfxStatus
        mainEntry(const char		*actionRaw,
                  const void		*handleRaw,
                  OfxPropertySetHandle	 inArgsRaw,
                  OfxPropertySetHandle	 outArgsRaw)
        {
            OFX::Log::print("********************************************************************************");
            OFX::Log::print("START mainEntry (%s)", actionRaw);
            OFX::Log::indent();
            OfxStatus stat = kOfxStatReplyDefault;
            try {
                // Cast the raw handle to be an image effect handle, because that is what it is
                OfxImageEffectHandle handle = (OfxImageEffectHandle) handleRaw;

                // Turn the arguments into wrapper objects to make our lives easier
                OFX::PropertySet inArgs(inArgsRaw);
                OFX::PropertySet outArgs(outArgsRaw);
    
                // turn the action into a std::string
                std::string action(actionRaw);

                // figure the actions
                if (action == kOfxActionLoad) {
                    // call the support load function, param-less
                    OFX::Private::loadAction(); 
      
                    // call the plugin side load action, param-less
                    OFX::Plugin::loadAction();

                    // got here, must be good
                    stat = kOfxStatOK;
                }

                // figure the actions
                else if (action == kOfxActionUnload) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, true, true, true);

                    // call the support load function, param-less
                    OFX::Private::unloadAction(); 
      
                    // call the plugin side unload action, param-less, should be called, eve if the stat above failed!
                    OFX::Plugin::unloadAction();

                    // got here, must be good
                    stat = kOfxStatOK;
                }

                else if(action == kOfxActionDescribe) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);

                    // make the plugin descriptor
                    ImageEffectDescriptor desc(handle);

                    // validate the host
                    OFX::Validation::validatePluginDescriptorProperties(fetchEffectProps(handle));

                    //  and pass it to the plugin to do something with it
                    OFX::Plugin::describe(desc);

                    // got here, must be good
                    stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionDescribeInContext) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);

                    // make the plugin descriptor and pass it to the plugin to do something with it
                    ImageEffectDescriptor desc(handle);
                    OFX::Plugin::describe(desc);

                    // figure the context and map it to an enum
                    std::string contextStr = inArgs.propGetString(kOfxImageEffectPropContext);
                    ContextEnum context = mapToContextEnum(contextStr);

                    // validate the host
                    OFX::Validation::validatePluginDescriptorProperties(fetchEffectProps(handle));

                    // call plugin descibe in context
                    OFX::Plugin::describeInContext(desc, context);

                    // got here, must be good
                    stat = kOfxStatOK;
                }
                else if(action == kOfxActionCreateInstance) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);
	  
                    // fetch the effect props to figure the context
                    PropertySet effectProps = fetchEffectProps(handle);

                    // get the context and turn it into an enum
                    std::string str = effectProps.propGetString(kOfxImageEffectPropContext);
                    ContextEnum context = mapToContextEnum(str);

                    // validate the plugin handle's properties
                    OFX::Validation::validatePluginInstanceProperties(fetchEffectProps(handle));

                    // make the image effect instance for this context
                    ImageEffect *instance = OFX::Plugin::createInstance(handle, context);

                    // got here, must be good
                    stat = kOfxStatOK;
                }
                else if(action == kOfxActionDestroyInstance) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);

                    // kill it
                    delete instance;

                    // got here, must be good
                    stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionRender) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);
	  
                    // call the render action skin
                    renderAction(handle, inArgs);

                    // got here, must be good
                    stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionBeginSequenceRender) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);

                    // call the begin render action skin
                    beginSequenceRenderAction(handle, inArgs);
                }
                else if(action == kOfxImageEffectActionEndSequenceRender) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);

                    // call the begin render action skin
                    endSequenceRenderAction(handle, inArgs);
                }
                else if(action == kOfxImageEffectActionIsIdentity) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, false);

                    // call the identity action, if it is, return OK
                    if(isIdentityAction(handle, inArgs, outArgs))
                        stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionGetRegionOfDefinition) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, false);

                    // call the rod action, return OK if it does something
                    if(regionOfDefinitionAction(handle, inArgs, outArgs))
                        stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionGetRegionsOfInterest) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, false);

                    // call the RoI action, return OK if it does something
                    if(regionsOfInterestAction(handle, inArgs, outArgs))
                        stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionGetFramesNeeded) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, false);

                    // call the frames needed action, return OK if it does something
                    if(framesNeededAction(handle, inArgs, outArgs))
                        stat = kOfxStatOK;
                }
                else if(action == kOfxImageEffectActionGetClipPreferences) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, false);

                    // call the frames needed action, return OK if it does something
                    if(clipPreferencesAction(handle, outArgs))
                        stat = kOfxStatOK;
                }
                else if(action == kOfxActionPurgeCaches) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);

                    // purge 'em
                    instance->purgeCaches();
                }
                else if(action == kOfxActionSyncPrivateData) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);

                    // and sync it
                    instance->syncPrivateData();
                }
                else if(action == kOfxImageEffectActionGetTimeDomain) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, false);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);

                    // we can only be a general context effect, so check that this is true
                    OFX::Log::error(instance->context() != eContextGeneral, "Calling kOfxImageEffectActionGetTimeDomain on an effect that is not a general context effect.");
                }
                else if(action == kOfxActionInstanceChanged) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);
                }
                else if(action == kOfxActionBeginInstanceChanged) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);
                }
                else if(action == kOfxActionEndInstanceChanged) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, false, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);
                }
                else if(action == kOfxActionBeginInstanceEdit) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);
                }
                else if(action == kOfxActionEndInstanceEdit) {
                    checkMainHandles(actionRaw, handleRaw, inArgsRaw, outArgsRaw, false, true, true);

                    // fetch our pointer out of the props on the handle
                    ImageEffect *instance = retrieveImageEffectPointer(handle);
                }
                else if(actionRaw) {
                    OFX::Log::error(true, "Unknown action '%s'.", actionRaw);
                }
                else {
                    OFX::Log::error(true, "Requested action was a null pointer.");
                }
            }

            // catch suite exceptions
            catch (OFX::Exception::Suite &ex)
            {
                stat = ex.status();
            }

            // catch host inadequate exceptions
            catch (OFX::Exception::HostInadequate &ex)
            {
                stat = kOfxStatErrMissingHostFeature;
            }

            // catch exception due to a property being unknown to the host, implies something wrong with host if not caught further down
            catch (OFX::Exception::PropertyUnknownToHost &ex)
            {
                stat = kOfxStatErrMissingHostFeature;
            }

            // catch memory
            catch (std::bad_alloc)
            {
                stat = kOfxStatErrMemory;
            }

            // Catch anything else, unknown
            catch (...)
            {
                stat = kOfxStatFailed;
            }
      
            OFX::Log::outdent();
            OFX::Log::print("STOP mainEntry (%s)\n", actionRaw);
            return stat;
        }      


        /** @brief The plugin function that gets passed the host structure. */
        void setHost(OfxHost *host)
        {
            gHost = host;
        }
    
    }; // namespace Private

    /** @brief namespace for memory allocation that is done via wrapping the ofx memory suite */
    namespace Memory {
        /** @brief allocate n bytes, returns a pointer to it */
        void *alloc(size_t nBytes, void *handleToAssociateWithTheAllocation) throw(std::bad_alloc)
        {
            void *data = 0;
            OfxStatus stat = OFX::Private::gMemorySuite->memoryAlloc(handleToAssociateWithTheAllocation, nBytes, &data);
            if(stat != kOfxStatOK)
                throw std::bad_alloc();
            return data;
        }

        /** @brief free n previously allocated memory */
        void free(void *ptr) throw()
        {
            // note we are ignore errors, this could be bad, but we don't throw on a destruction
            OFX::Private::gMemorySuite->memoryFree(ptr);            
        }

    };

}; // namespace OFX



/** @brief, mandated function returning the number of plugins, which is always 1 */
OfxExport int 
OfxGetNumberOfPlugins(void)
{
  return 1;
}

/** @brief, mandated function returning the nth plugin 

  We call the plugin side defined OFX::Plugin::getPluginID function to find out what to set.
*/
OfxExport OfxPlugin *
OfxGetPlugin(int nth)
{
  OFX::Log::error(nth != 0, "Host attempted to get plugin %d, when there is only 1 plugin, so it should have asked for 0", nth);
  // the raw OFX plugin struct returned to the host
  static OfxPlugin      ofxPlugin;

  // struct identifying the plugin to the support lib 
  static OFX::PluginID id;

  // whether we have done this before
  static bool gotID = false;

  if(!gotID) {
    gotID = true;

    // API is always an image effect plugin
    ofxPlugin.pluginApi  = kOfxImageEffectPluginApi;
    ofxPlugin.apiVersion = 1;

    // call the plugin defined id function
    OFX::Plugin::getPluginID(id);

    // set identifier, version major and version minor 
    ofxPlugin.pluginIdentifier   = id.pluginIdentifier.c_str();
    ofxPlugin.pluginVersionMajor = id.pluginVersionMajor;
    ofxPlugin.pluginVersionMinor = id.pluginVersionMinor;

    // set up our two routines
    ofxPlugin.setHost    = OFX::Private::setHost;
    ofxPlugin.mainEntry  = OFX::Private::mainEntry;
  }

  return &ofxPlugin;
}
