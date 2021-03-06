/* Copyright 2014 - 2015 CyberTech Labs Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "coreDumping.h"

#include <QtCore/QDir>

#include <sys/resource.h>
#include <signal.h>

#include "paths.h"

void (*oldHandler)(int);

void dumpHandler(int signal)
{
	QDir::setCurrent(trikKernel::Paths::coreDumpPath());
	oldHandler(signal);
}

void initSignals()
{
	QList<int> const signalsList({SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGSEGV, SIGBUS, SIGSYS, SIGTRAP, SIGXCPU, SIGXFSZ
			, SIGIOT});

	struct sigaction oldAction;
	sigaction(SIGSEGV, nullptr, &oldAction);
	oldHandler = oldAction.sa_handler;

	struct sigaction action;
	action.sa_handler = dumpHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	for (int signal : signalsList) {
		sigaddset(&action.sa_mask, signal);
		sigaction(signal, &action, nullptr);
	}
}

void setCoreLimits()
{
	rlimit core_limits;
	core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &core_limits);
}

void trikKernel::coreDumping::initCoreDumping()
{
	initSignals();
	setCoreLimits();
}
