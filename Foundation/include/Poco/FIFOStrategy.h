//
// FIFOStrategy.h
//
// $Id: //poco/1.3/Foundation/include/Poco/FIFOStrategy.h#4 $
//
// Library: Foundation
// Package: Events
// Module:  FIFOStragegy
//
// Implementation of the FIFOStrategy template.
//
// Copyright (c) 2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#ifndef  Foundation_FIFOStrategy_INCLUDED
#define  Foundation_FIFOStrategy_INCLUDED


#include "Poco/NotificationStrategy.h"
#include <map>
#include <list>
#include <memory>


namespace Poco {


template <class TArgs, class TDelegate, class TCompare> 
class FIFOStrategy: public NotificationStrategy<TArgs, TDelegate>
{
public:
	typedef std::list<TDelegate*>                    Delegates;
	typedef typename Delegates::iterator             Iterator;
	typedef typename Delegates::const_iterator       ConstIterator;
	typedef std::map<TDelegate*, Iterator, TCompare> DelegateIndex;
	typedef typename DelegateIndex::iterator         IndexIterator;
	typedef typename DelegateIndex::const_iterator   ConstIndexIterator;

	FIFOStrategy()
	{
	}

	FIFOStrategy(const FIFOStrategy& s)
	{
		operator = (s);
	}

	~FIFOStrategy()
	{
		clear();
	}

	void notify(const void* sender, TArgs& arguments)
	{
		std::vector<Iterator> delMe;
		Iterator it    = _observers.begin();
		Iterator itEnd = _observers.end();

		for (; it != itEnd; ++it)
		{
			if (!(*it)->notify(sender, arguments))
			{
				// schedule for deletion
				delMe.push_back(it);
			}
		}
		
		while (!delMe.empty())
		{
			typename std::vector<Iterator>::iterator vit = delMe.end();
			--vit;
			delete **vit;
			_observers.erase(*vit);
			delMe.pop_back();
		}
	}

	void add(const TDelegate& delegate)
	{
		IndexIterator it = _observerIndex.find(const_cast<TDelegate*>(&delegate));
		if (it != _observerIndex.end())
		{
			delete *it->second;
			_observers.erase(it->second);
			_observerIndex.erase(it);
		}
		std::auto_ptr<TDelegate> pDelegate(delegate.clone());
		_observers.push_back(pDelegate.get());
		bool tmp = _observerIndex.insert(make_pair(pDelegate.get(), --_observers.end())).second;
		poco_assert (tmp);
		pDelegate.release();
	}

	void remove(const TDelegate& delegate)
	{
		IndexIterator it = _observerIndex.find(const_cast<TDelegate*>(&delegate));

		if (it != _observerIndex.end())
		{
			delete *it->second;
			_observers.erase(it->second);
			_observerIndex.erase(it);
		}
	}

	FIFOStrategy& operator = (const FIFOStrategy& s)
	{
		if (this != &s)
		{
			for (ConstIterator it = s._observers.begin(); it != s._observers.end(); ++it)
			{
				add(**it);
			}
		}
		return *this;
	}

	void clear()
	{
		for (Iterator it = _observers.begin(); it != _observers.end(); ++it)
		{
			delete *it;
		}

		_observers.clear();
		_observerIndex.clear();
	}

	bool empty() const
	{
		return _observers.empty();
	}

protected:
	Delegates     _observers;     /// Stores the delegates in the order they were added.
	DelegateIndex _observerIndex; /// For faster lookup when add/remove is used.
};


} // namespace Poco


#endif
