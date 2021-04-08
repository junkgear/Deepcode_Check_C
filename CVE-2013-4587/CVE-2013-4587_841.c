static int kvm_vm_ioctl_create_vcpu(struct kvm *kvm, u32 id)
{
	int r;
	struct kvm_vcpu *vcpu, *v;

	vcpu = kvm_arch_vcpu_create(kvm, id);
	if (IS_ERR(vcpu))
		return PTR_ERR(vcpu);

	preempt_notifier_init(&vcpu->preempt_notifier, &kvm_preempt_ops);

	r = kvm_arch_vcpu_setup(vcpu);
	if (r)
		return r;

	mutex_lock(&kvm->lock);
	if (atomic_read(&kvm->online_vcpus) == KVM_MAX_VCPUS) {
		r = -EINVAL;
		goto vcpu_destroy;
	}

	kvm_for_each_vcpu(r, v, kvm)
		if (v->vcpu_id == id) {
			r = -EEXIST;
			goto vcpu_destroy;
		}

	BUG_ON(kvm->vcpus[atomic_read(&kvm->online_vcpus)]);

	/* Now it's all set up, let userspace reach it */
	kvm_get_kvm(kvm);
	r = create_vcpu_fd(vcpu);
	if (r < 0) {
		kvm_put_kvm(kvm);
		goto vcpu_destroy;
	}

	kvm->vcpus[atomic_read(&kvm->online_vcpus)] = vcpu;
	smp_wmb();
	atomic_inc(&kvm->online_vcpus);

#ifdef CONFIG_KVM_APIC_ARCHITECTURE
	if (kvm->bsp_vcpu_id == id)
		kvm->bsp_vcpu = vcpu;
#endif
	mutex_unlock(&kvm->lock);
	return r;

vcpu_destroy:
	mutex_unlock(&kvm->lock);
	kvm_arch_vcpu_destroy(vcpu);
	return r;
}