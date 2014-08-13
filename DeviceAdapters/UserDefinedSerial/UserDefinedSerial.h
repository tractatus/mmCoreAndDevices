// DESCRIPTION:   Control devices using user-specified serial commands
//
// COPYRIGHT:     University of California San Francisco, 2014
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// AUTHOR:        Mark Tsuchida

#pragma once

#include "DeviceBase.h"
#include "DeviceUtils.h"

#include <boost/scoped_array.hpp>
#include <boost/utility.hpp>

#include <memory>
#include <string>
#include <vector>


// Make sure to add message to UserDefSerialBase::RegisterErrorMessages() when
// adding a new code.
const int ERR_BINARY_SERIAL_TIMEOUT = 107; // Use the famous SerialManager code
const int ERR_UNEXPECTED_RESPONSE = 2001;
const int ERR_QUERY_COMMAND_EMPTY = 2002;
const int ERR_ASCII_COMMAND_CONTAINS_NULL = 2003;
const int ERR_TRAILING_BACKSLASH = 2004;
const int ERR_UNKNOWN_ESCAPE_SEQUENCE = 2005;
const int ERR_EMPTY_HEX_ESCAPE_SEQUENCE = 2006;
const int ERR_CANNOT_GET_PORT_TIMEOUT = 2007;
const int ERR_CANNOT_QUERY_IN_IGNORE_MODE = 2008;
const int ERR_EXPECTED_RESPONSE_LENGTH_MISMATCH = 2009;
const int ERR_NO_RESPONSE_ALTERNATIVES = 2010;
const int ERR_VAR_LEN_RESPONSE_MUST_NOT_BE_EMPTY = 2011;


/**
 * \brief Convert bytes to C-style escaped string.
 */
std::string EscapedStringFromByteString(const std::vector<char>& bytes);

/**
 * \brief Convert C-style escaped string to bytes.
 */
int ByteStringFromEscapedString(const std::string& escaped,
      std::vector<char>& bytes);


/**
 * \brief Interface for serial response detection.
 */
class ResponseDetector : boost::noncopyable
{
public:
   static std::auto_ptr<ResponseDetector> NewByName(const std::string& name);

   virtual ~ResponseDetector() {}

   virtual std::string GetMethodName() const = 0;

   /**
    * \brief Receive and match to an expected response.
    * \return error code if could not receive or did not match
    */
   virtual int RecvExpected(MM::Core* core, MM::Device* device,
         const std::string& port, const std::vector<char>& expected) = 0;

   /**
    * \brief Receive and match to one of a number of possible responses.
    */
   virtual int RecvAlternative(MM::Core* core, MM::Device* device,
         const std::string& port,
         const std::vector< std::vector<char> >& alternatives,
         size_t& index) = 0;
};

class IgnoringResponseDetector : public ResponseDetector
{
public:
   static std::auto_ptr<ResponseDetector> NewByName(const std::string& name);

   virtual std::string GetMethodName() const;
   virtual int RecvExpected(MM::Core* core, MM::Device* device,
         const std::string& port, const std::vector<char>& expected);
   virtual int RecvAlternative(MM::Core* core, MM::Device* device,
         const std::string& port,
         const std::vector< std::vector<char> >& alternatives,
         size_t& index);

private:
   IgnoringResponseDetector() {}
};

class TerminatorResponseDetector : public ResponseDetector
{
   std::string terminator_;
   std::string terminatorName_;

public:
   static std::auto_ptr<ResponseDetector> NewByName(const std::string& name);

   virtual std::string GetMethodName() const;
   virtual int RecvExpected(MM::Core* core, MM::Device* device,
         const std::string& port, const std::vector<char>& expected);
   virtual int RecvAlternative(MM::Core* core, MM::Device* device,
         const std::string& port,
         const std::vector< std::vector<char> >& alternatives,
         size_t& index);

private:
   TerminatorResponseDetector(const char* terminator,
         const char* terminatorName) :
      terminator_(terminator), terminatorName_(terminatorName)
   {}
   int Recv(MM::Core* core, MM::Device* device, const std::string& port,
         std::vector<char>& response);
};

class BinaryResponseDetector : public ResponseDetector
{
protected:
   int Recv(MM::Core* core, MM::Device* device, const std::string& port,
         size_t recvLen, std::vector<char>& response);
};

class FixedLengthResponseDetector : public BinaryResponseDetector
{
   size_t byteCount_;

public:
   static std::auto_ptr<ResponseDetector> NewByName(const std::string& name);

   virtual std::string GetMethodName() const;
   virtual int RecvExpected(MM::Core* core, MM::Device* device,
         const std::string& port, const std::vector<char>& expected);
   virtual int RecvAlternative(MM::Core* core, MM::Device* device,
         const std::string& port,
         const std::vector< std::vector<char> >& alternatives,
         size_t& index);

private:
   FixedLengthResponseDetector(size_t byteCount) : byteCount_(byteCount) {}
};

class VariableLengthResponseDetector : public BinaryResponseDetector
{
public:
   static std::auto_ptr<ResponseDetector> NewByName(const std::string& name);

   virtual std::string GetMethodName() const;
   virtual int RecvExpected(MM::Core* core, MM::Device* device,
         const std::string& port, const std::vector<char>& expected);
   virtual int RecvAlternative(MM::Core* core, MM::Device* device,
         const std::string& port,
         const std::vector< std::vector<char> >& alternatives,
         size_t& index);

private:
   VariableLengthResponseDetector() {}
};


/**
 * \brief Common base class template for concrete device classes.
 *
 * Uses the template template argument TBasicDevice so that it can inherit from
 * either CShutterBase CStateDeviceBase (or other).
 *
 * \tparam TBasicDevice the device base class
 * \tparam UConcreteDevice the concrete device class
 */
template <template <class> class TBasicDevice, class UConcreteDevice>
class UserDefSerialBase : public TBasicDevice<UConcreteDevice>
{
protected:
   typedef UserDefSerialBase Self;
   typedef TBasicDevice<UConcreteDevice> Super;
   typedef UConcreteDevice DeviceType;
   // An always-safe cast
   DeviceType* This() { return static_cast<DeviceType*>(this); }

public:
   UserDefSerialBase();
   virtual ~UserDefSerialBase() {}

   // MM::Device methods
   // Derived classes must call these base class versions if overriding
   virtual int Initialize();
   virtual int Shutdown();
   virtual bool Busy(); // Should not be overridden

private:
   // Register error messages for all derived devices (we use common codes
   // within this device adapter)
   void RegisterErrorMessages();
   // No point in making virtual (called from ctor)
   void CreatePreInitProperties();

protected:
   // Derived classes must call this base class version if overriding
   virtual int CreatePostInitProperties();
   virtual void StartBusy(); // Should not be overridden

protected:
   // Property action handlers
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnCommandSendMode(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnResponseDetectionMethod(MM::PropertyBase* pProp, MM::ActionType eAct);

protected:
   // Create a string property for a command or response string, mapped to
   // an instance variable
   int CreateByteStringProperty(const char* name,
         std::vector<char>& varRef, bool preInit = false);
   int CreateByteStringProperty(const std::string& name,
         std::vector<char>& varRef, bool preInit = false)
   { return CreateByteStringProperty(name.c_str(), varRef, preInit); }

   int SendRecv(const std::vector<char>& command,
         const std::vector<char>& expectedResponse);

   // Send a command and match response against several alternatives
   int SendQueryRecvAlternative(const std::vector<char>& command,
         const std::vector< std::vector<char> >& responseAlts,
         size_t& responseAltIndex);

private:
   int Send(const std::vector<char>& command);

private:
   std::string port_;
   bool initialized_;

   MM::MMTime lastActionTime_;

   bool binaryMode_;
   std::string asciiTerminator_;
   std::auto_ptr<ResponseDetector> responseDetector_;

   std::vector<char> initializeCommand_;
   std::vector<char> initializeResponse_;
   std::vector<char> shutdownCommand_;
   std::vector<char> shutdownResponse_;
};


class UserDefSerialShutter :
   public UserDefSerialBase<CShutterBase, UserDefSerialShutter>
{
protected:
   typedef UserDefSerialShutter Self;
   typedef UserDefSerialBase< ::CShutterBase, UserDefSerialShutter > Super;

public:
   UserDefSerialShutter();

   virtual void GetName(char* name) const;

   // MM::Shutter methods
   virtual int SetOpen(bool open);
   virtual int GetOpen(bool& open);
   virtual int Fire(double) { return DEVICE_UNSUPPORTED_COMMAND; }

   // Overrides
   virtual int Initialize();
   virtual int Shutdown();

private:
   void CreatePreInitProperties();

protected:
   // Override
   virtual int CreatePostInitProperties();

private:
   // Property action handlers
   int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   bool lastSetOpen_;

   std::vector<char> openCommand_;
   std::vector<char> openResponse_;
   std::vector<char> closeCommand_;
   std::vector<char> closeResponse_;
   std::vector<char> queryCommand_;
   std::vector<char> queryOpenResponse_;
   std::vector<char> queryCloseResponse_;
};


class UserDefSerialStateDevice :
   public UserDefSerialBase<CStateDeviceBase, UserDefSerialStateDevice>
{
protected:
   typedef UserDefSerialStateDevice Self;
   typedef UserDefSerialBase< ::CStateDeviceBase, UserDefSerialStateDevice >
      Super;

public:
   UserDefSerialStateDevice();

   virtual void GetName(char* name) const;

   // MM::State methods (Get/SetState() is implemented in CStateDeviceBase
   // based on the State property)
   virtual unsigned long GetNumberOfPositions() const;

private:
   void CreatePreInitProperties();

protected:
   // Override
   virtual int CreatePostInitProperties();

private:
   // Property action handlers
   int OnNumberOfPositions(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnState(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   size_t numPositions_;
   size_t currentPosition_;

   boost::scoped_array< std::vector<char> > positionCommands_;
   boost::scoped_array< std::vector<char> > positionResponses_;
   std::vector<char> queryCommand_;
   boost::scoped_array< std::vector<char> > queryResponses_;
};