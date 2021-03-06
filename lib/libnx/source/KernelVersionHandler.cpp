#include <nx/KernelVersionHandler.h>

nx::KernelVersionHandler::KernelVersionHandler() :
	mIsSet(false),
	mEntry(0,0)
{}

void nx::KernelVersionHandler::operator=(const KernelVersionHandler & other)
{
	mIsSet = other.mIsSet;
	mEntry.setKernelCapability(other.mEntry.getKernelCapability());
}

bool nx::KernelVersionHandler::operator==(const KernelVersionHandler & other) const
{
	return (mIsSet == other.mIsSet) \
		&& (mEntry.getKernelCapability() == other.mEntry.getKernelCapability());
}

bool nx::KernelVersionHandler::operator!=(const KernelVersionHandler & other) const
{
	return !(*this == other);
}

void nx::KernelVersionHandler::importKernelCapabilityList(const fnd::List<KernelCapabilityEntry>& caps)
{
	if (caps.size() > kMaxKernelCapNum)
	{ 
		throw fnd::Exception(kModuleName, "Too many kernel capabilities");
	}

	if (caps.size() == 0)
		return;

	mEntry.setKernelCapability(caps[0]);

	mIsSet = true;
}

void nx::KernelVersionHandler::exportKernelCapabilityList(fnd::List<KernelCapabilityEntry>& caps) const
{
	if (isSet() == false)
		return;

	caps.addElement(mEntry.getKernelCapability());
}

void nx::KernelVersionHandler::clear()
{
	mIsSet = false;
	mEntry.setVerMajor(0);
	mEntry.setVerMinor(0);
}

bool nx::KernelVersionHandler::isSet() const
{
	return mIsSet;
}

uint16_t nx::KernelVersionHandler::getVerMajor() const
{
	return mEntry.getVerMajor();
}

void nx::KernelVersionHandler::setVerMajor(uint16_t major)
{
	mEntry.setVerMajor(major);
	mIsSet = true;
}

uint8_t nx::KernelVersionHandler::getVerMinor() const
{
	return mEntry.getVerMinor();
}

void nx::KernelVersionHandler::setVerMinor(uint8_t minor)
{
	mEntry.setVerMinor(minor);
	mIsSet = true;
}