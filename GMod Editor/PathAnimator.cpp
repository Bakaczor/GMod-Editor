#include "PathAnimator.h"

using namespace app;

PathAnimator::PathAnimator(Milling& milling) : m_milling(milling) {}

void PathAnimator::StartAnimation() {
	isRunning = true;
}

void PathAnimator::StopAnimation() {
	isRunning = false;
}

void PathAnimator::RestartAnimation() {
	// TODO
}

void PathAnimator::CompleteAnimation() {
	// TODO
}
